//Qt
#include <QCoreApplication>

//My
#include "TradingCatCommon/types.h"

#include "TradingCatCommon/dbloader.h"

using namespace TradingCatCommon;
using namespace Common;

DBLoader::DBLoader(const Common::DBConnectionInfo& dbConnectionInfo, QObject *parent)
    : QObject{parent}
    , _dbConnectionInfo(dbConnectionInfo)
{
    Q_ASSERT(_dbConnectionInfo.check().isEmpty());
}

DBLoader::~DBLoader()
{
    stop();
}

void DBLoader::start()
{
    Q_ASSERT(!_isStarted);

    try
    {
        connectToDB(_db, _dbConnectionInfo, QString("%1_DBLoader_DB").arg(QCoreApplication::applicationName()));
    }
    catch (const SQLException& err)
    {
        emit errorOccurred(EXIT_CODE::SQL_NOT_CONNECT, err.what());

        return;
    }

    _isStarted = true;
}

void DBLoader::stop()
{
    if (!_isStarted)
    {
        return;
    }

    closeDB(_db);

    _isStarted = false;
}

void DBLoader::loadMoneyMap()
{
    const auto queryText =
        QString("SELECT `ID`, `StockExchange`, `Money`, `KLineInterval`, `LastCloseDateTime` "
                "FROM %1.`KLines`")
            .arg(_db.databaseName());

    try
    {
        transactionDB(_db);

        QSqlQuery query(_db);
        query.setForwardOnly(true);

        DBQueryExecute(_db, query, queryText);

        _moneyMap.clear();

        const auto currDateTime = QDateTime::currentDateTime();
        while(query.next())
        {
            const auto lastClose = query.value("lastCloseDateTime").toDateTime();

            if (lastClose.daysTo(currDateTime) <= 30)
            {
                addMoneyInfo(query.value("StockExchange").toString(),
                         query.value("Money").toString(),
                         query.value("KLineInterval").toString(),
                         query.value("lastCloseDateTime").toDateTime(),
                         query.value("ID").toULongLong());
            }
        }

        commitDB(_db);
    }
    catch (const SQLException& err)
    {
        emit errorOccurred(EXIT_CODE::SQL_EXECUTE_QUERY_ERR, err.what());
    }
}

void DBLoader::addMoneyInfo(const QString &stockExchange, const QString &symbol, const QString &type, const QDateTime& lastClose, quint64 id)
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

    MoneyInfo tmp;
    tmp.klineID = std::move(klineID);
    tmp.stockExchangeID = std::move(stockExchangeID);
    tmp.lastClose = lastClose;

    _moneyMap.emplace(id, std::move(tmp));
}

const DBLoader::MoneyInfo &DBLoader::getMoneyInfo(quint64 id) const
{
    return _moneyMap.at(id);
}

std::unique_ptr<TradingCatCommon::KLinesDataContainer> DBLoader::loadFromDB()
{
    // Q_ASSERT(_db.isOpen());

    // loadMoneyMap();

    // auto p_TradingData = std::make_unique<KLinesDataContainer>();

    // if (_moneyMap.empty())
    // {
    //     return p_TradingData;
    // }

    // const auto openDateTime =QDateTime::currentDateTime().addDays(-1);
    // for (const auto& [moneyId, money]: _moneyMap)
    // {
    //     auto kline = std::make_shared<KLine>();

    //     kline->id = money.klineID;
    //     kline->openTime = openDateTime;
    //     kline->closeTime = openDateTime.addMSecs(static_cast<qint64>(money.klineID.type));
    //     kline->open = 0.0f;
    //     kline->close = 0.0f;
    //     kline->high = 0.0f;
    //     kline->low = 0.0f;
    //     kline->volume = 0.0f;
    //     kline->quoteAssetVolume = 0.0f;

    //     auto klinesList =std::make_shared<KLinesList>();
    //     klinesList->emplace_back(std::move(kline));

    //     p_TradingData->addKLines(money.stockExchangeID, klinesList);
    // }

    // emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, "Loaded old kline from DB complited successfully");

    // return p_TradingData;
}
