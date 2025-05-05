//Qt
#include <QSqlQuery>
#include <QCoreApplication>

//My
#include <Common/sql.h>

#include "TradingCatCommon/dbsaver.h"

using namespace TradingCatCommon;
using namespace Common;

DBSaver::DBSaver(const Common::DBConnectionInfo& dbConnectionInfo, QObject *parent)
    : QObject{parent}
    , _dbConnectionInfo(dbConnectionInfo)
{
    Q_ASSERT(_dbConnectionInfo.check().isEmpty());
}

DBSaver::~DBSaver()
{
    stop();
}

void DBSaver::start()
{
    Q_ASSERT(!_isStarted);

    try
    {
        connectToDB(_db, _dbConnectionInfo, QString("%1_DBSaver_Service_DB").arg(QCoreApplication::applicationName()));
        connectToDB(_dbSaver, _dbConnectionInfo, QString("%1_DBSaver_DB").arg(QCoreApplication::applicationName()));
    }
    catch (const SQLException& err)
    {
        emit errorOccurred(EXIT_CODE::SQL_NOT_CONNECT, err.what());

        return;
    }

    loadKLineTables();

    _isStarted = true;
}

void DBSaver::stop()
{
    if (!_isStarted)
    {
        return;
    }

    _saveResult = std::async(&DBSaver::saveToDB, std::ref(_dbSaver)/*, std::move(_insertQuery)*/, std::move(_updateQuery));
    checkResult();

    closeDB(_dbSaver);
    closeDB(_db);

    _isStarted = false;

    emit finished();
}

void DBSaver::saveKLines(const StockExchangeID& id, const PKLinesList& klinesList)
{
    Q_ASSERT(_isStarted);
    Q_ASSERT(!klinesList->empty());

    const auto curDateTime = QDateTime::currentDateTime().toString(DATETIME_FORMAT);

    KLinesList localKLinesList(*klinesList);

    //Сортируем данные, если они все еще не отсортированы
    localKLinesList.sort(
        [](const auto& kline1, const auto& kline2)
        {
            return kline1->closeTime < kline2->closeTime;
        });

    const auto kline = *localKLinesList.rbegin();
    const auto tableName = getKLineTableName(id.name, kline->id.symbol, KLineTypeToString(kline->id.type));

    auto moneyID = checkKLineTable(id, kline->id);

    Q_ASSERT(moneyID != 0);

    const QString closeDateTime = kline->closeTime.toString(DATETIME_FORMAT);

    const auto updateText =
    QString("UPDATE %1.`KLines` "
                "SET `LastCloseDateTime` = '%2' "
                "WHERE ID = %3")
            .arg(_db.databaseName())
            .arg(closeDateTime)
            .arg(moneyID);

    if (!_updateQuery)
    {
        _updateQuery = std::make_unique<QueueMessages>();
    }
    _updateQuery->emplace(std::move(updateText));


    if (_updateQuery && _updateQuery->size() > 500)
    {
        checkResult();

        auto updateQueueMessages = std::move(_updateQuery);

        _updateQuery.reset();

        _saveResult = std::async(&DBSaver::saveToDB, std::ref(_dbSaver)/*, std::move(insertQueueMessages)*/, std::move(updateQueueMessages));
    }
}

void DBSaver::loadKLineTables()
{
    const auto queryText =
        QString("SELECT `ID`, `StockExchange`, `Money`, `KLineInterval`, `LastCloseDateTime`"
                "FROM %1.`KLines`")
            .arg(_db.databaseName());

    try
    {
        transactionDB(_db);

        QSqlQuery query(_db);
        query.setForwardOnly(true);

        DBQueryExecute(_db, query, queryText);

        _klineTables.clear();
        _tablesList.clear();
        while(query.next())
        {
            const auto stockExchange = query.value("StockExchange").toString();
            const auto money = query.value("Money").toString();
            const auto interval = query.value("KLineInterval").toString();
            const auto id = query.value("ID").toULongLong();

            addMoneyID(stockExchange, money, interval, id);

            _tablesList.insert(getKLineTableName(stockExchange, money, interval));;
        }

        commitDB(_db);
    }
    catch (const SQLException& err)
    {
        emit errorOccurred(EXIT_CODE::SQL_EXECUTE_QUERY_ERR, err.what());
    }
}

quint64 DBSaver::checkKLineTable(const StockExchangeID& stockExcangeID, const KLineID& klineID)
{
    auto id = getMoneyID(stockExcangeID, klineID);
    if (id == 0)
    {
        const auto queryText =
            QString("INSERT DELAYED INTO %1.`KLines` (`StockExchange`, `Money`, `KLineInterval`, `LastCloseDateTime`) "
                    "VALUES('%2', '%3', '%4', '%5') ")
                .arg(_db.databaseName())
                .arg(stockExcangeID.name)
                .arg(klineID.symbol)
                .arg(KLineTypeToString(klineID.type))
                .arg(QDateTime::currentDateTime().toString(DATETIME_FORMAT));

        try
        {
            DBQueryExecute(_db, queryText);
        }
        catch (const SQLException& err)
        {
            emit errorOccurred(EXIT_CODE::SQL_EXECUTE_QUERY_ERR, err.what());

            return 0;
        }

        emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, QString("Added new kline %1 for stock exchange %2")
                                                                  .arg(klineID.toString())
                                                                  .arg(stockExcangeID.toString()));

        loadKLineTables();

        id = loadIDFromDB(stockExcangeID, klineID);
    }

    Q_ASSERT(id != 0);

    return id;
}

quint64 DBSaver::getMoneyID(const StockExchangeID &stockExchangeID, const KLineID& klineID) const
{
    const auto klineTables_it = _klineTables.find(stockExchangeID);
    if (klineTables_it == _klineTables.end())
    {
        return 0;
    }

    const auto& money = klineTables_it.value();
    const auto money_it = money.find(klineID);
    if (money_it == money.end())
    {
        return 0;
    }

    return money_it.value();
}

void DBSaver::addMoneyID(const QString& stockExchange, const QString& symbol, const QString& type, quint64 id)
{
    Q_ASSERT(!stockExchange.isEmpty());
    Q_ASSERT(!symbol.isEmpty());
    Q_ASSERT(!type.isEmpty());
    Q_ASSERT(id != 0);

    StockExchangeID stockExchangeID;
    stockExchangeID.name = stockExchange;

    KLineID klineID;
    klineID.symbol = symbol;
    klineID.type = stringToKLineType(type);

    auto klineTables_it = _klineTables.find(stockExchangeID);
    if (klineTables_it == _klineTables.end())
    {
        MoneyMap tmp;
        tmp.insert(klineID, id);

        _klineTables.insert(stockExchangeID, tmp);
    }
    else
    {
        klineTables_it.value().insert(klineID, id);
    }
}

 std::optional<QString> DBSaver::saveToDB(QSqlDatabase& db, PQueueMessages updateQueueMessages)
{
    try
    {
        transactionDB(db);
        QSqlQuery query(db);

        while (!updateQueueMessages->empty())
        {
            const auto& queryText = updateQueueMessages->front();

            DBQueryExecute(db, query, queryText);

            updateQueueMessages->pop();
        }

        commitDB(db);
    }
    catch (const SQLException& err)
    {
        return err.what();
    }

    return std::nullopt;
}

void DBSaver::checkResult()
{
    if (!_saveResult.valid())
    {
        return;
    }

    const auto errMsg = _saveResult.get();
    if (errMsg.has_value())
    {
        emit errorOccurred(EXIT_CODE::SQL_EXECUTE_QUERY_ERR, errMsg.value());
    }
}

quint64 DBSaver::loadIDFromDB(const StockExchangeID& stockExcangeID, const KLineID& klineID)
{
    const auto intervalStr = KLineTypeToString(klineID.type);

    const auto queryText =
        QString("SELECT `ID`, `StockExchange`, `Money`, `KLineInterval`, `LastCloseDateTime` "
                "FROM %1.`KLines`"
                "WHERE `StockExchange` = '%2' AND `Money` = '%3' AND `KLineInterval` = '%4' ")
            .arg(_db.databaseName())
            .arg(stockExcangeID.name)
            .arg(klineID.symbol)
            .arg(intervalStr);

    quint64 id = 0;

    try
    {
        transactionDB(_db);

        QSqlQuery query(_db);
        query.setForwardOnly(true);

        DBQueryExecute(_db, query, queryText);

        if (query.next())
        {
            id = query.value("ID").toULongLong();

            addMoneyID(stockExcangeID.name, klineID.symbol, intervalStr, id);

            _tablesList.insert(getKLineTableName(stockExcangeID.name, klineID.symbol, intervalStr));
        }

        commitDB(_db);
    }
    catch (const SQLException& err)
    {
        emit errorOccurred(EXIT_CODE::SQL_EXECUTE_QUERY_ERR, err.what());
    }

    return id;
}


