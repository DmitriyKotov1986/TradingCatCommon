//Qt
#include <QSqlQuery>
#include <QCoreApplication>

//My
#include <Common/sql.h>

#include <TradingCatCommon/kline.h>

#include "TradingCatCommon/dbcleaner.h"

using namespace TradingCatCommon;
using namespace Common;

static const qint64 CLEAR_INTERVAL = 60 * 60 * 1000;         ///< 1 hour - интервал выполнения очистки
static const qint64 SAVE_INTERVAL = 7 * 24 * 60 * 60 * 1000; ///< 1 week - глубина хранение данных

DBCleaner::DBCleaner(const Common::DBConnectionInfo& dbConnectionInfo, QObject *parent /* = nullptr */)
    : QObject{parent}
    , _dbConnectionInfo(dbConnectionInfo)
{
    Q_ASSERT(_dbConnectionInfo.check().isEmpty());
}

DBCleaner::~DBCleaner()
{
    stop();
}

void DBCleaner::start()
{
    Q_ASSERT(!_isStarted);

    try
    {
        connectToDB(_db, _dbConnectionInfo, QString("%1_DBCleaner_DB").arg(QCoreApplication::applicationName()));
    }
    catch (const SQLException& err)
    {
        emit errorOccurred(EXIT_CODE::SQL_NOT_CONNECT, err.what());

        return;
    }

    _timer = std::make_unique<QTimer>();

    QObject::connect(_timer.get(), SIGNAL(timeout()), SLOT(clear()));

    _timer->start(CLEAR_INTERVAL);

    clear();

    _isStarted = true;
}

void DBCleaner::stop()
{
    if (!_isStarted)
    {
        return;
    }

    _timer.reset();

    closeDB(_db);

    _isStarted = false;
}

void DBCleaner::clear()
{
    const auto olderDateTime = QDateTime::currentDateTime().addMSecs(-SAVE_INTERVAL).toString(DATETIME_FORMAT);

    try
    {
        transactionDB(_db);

        QSqlQuery query(_db);
        query.setForwardOnly(true);

        auto queryText =
            QString("SELECT `StockExchange`, `Money`, `KLineInterval`, `LastCloseDateTime` "
                    "FROM %1.`KLines`")
                .arg(_db.databaseName());

        DBQueryExecute(_db, query, queryText);

        QSet<QString> tablesList;
        while(query.next())
        {
            const auto stockExchange = query.value("StockExchange").toString();
            const auto money = query.value("Money").toString();
            const auto interval = query.value("KLineInterval").toString();

            tablesList.insert(getKLineTableName(stockExchange, money, interval));
        }

        for (const auto &tableName: tablesList)
        {
            auto queryText =
                QString("DELETE FROM %1.`%2` "
                        "WHERE `CloseDateTime` < '%3' ")
                    .arg(_db.databaseName())
                    .arg(tableName)
                    .arg(olderDateTime);

            DBQueryExecute(_db, query, queryText);
        }

        commitDB(_db);
    }
    catch (const SQLException& err)
    {
        emit errorOccurred(EXIT_CODE::SQL_EXECUTE_QUERY_ERR, err.what());
    }

    emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, QString("The DB has been successfully cleared. Last saved: %1").arg(olderDateTime));
}
