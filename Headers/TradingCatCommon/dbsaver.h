#pragma once

//STL
#include <queue>
#include <future>
#include <memory>

//QT
#include <QObject>
#include <QSqlDatabase>
#include <QQueue>

//My
#include <Common/common.h>
#include <Common/sql.h>
#include <Common/tdbloger.h>

#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     The DBSaver class сохраняет значения свечей в БД
///
class DBSaver final
    : public QObject
{
    Q_OBJECT

public:
    /*!
        Конструктор. Планируется использовать только этот конструтор
        @param dbConnectionInfo - параметры подключения к БД. Значение должно быть корректным
        @param parent - указатель на родительски класс
    */
    explicit DBSaver(const Common::DBConnectionInfo& dbConnectionInfo, QObject *parent = nullptr);

    /*!
        Деструктор
    */
    ~DBSaver();

public slots:
    void start();
    void stop();

    void saveKLines(const TradingCatCommon::StockExchangeID& id, const TradingCatCommon::PKLinesList& klines);

signals:
    /*!
        Сигнал генерируется если в процессе работы с БД произошла ошибка
        @param errorCode - код ошибки
        @param errorString - текстовое описание ошибки
    */
    void errorOccurred(Common::EXIT_CODE errorCode, const QString& errorString);

    /*!
        Дополнительное сообщение логеру
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsg(Common::TDBLoger::MSG_CODE category, const QString& msg);

    void finished();

private:
    // Удаляем неиспользуемые конструкторы
    DBSaver() = delete;
    Q_DISABLE_COPY_MOVE(DBSaver);

    void loadKLineTables();
    quint64 checkKLineTable(const TradingCatCommon::StockExchangeID& stockExchangeID, const TradingCatCommon::KLineID& klineID);
    void addMoneyID(const QString& stockExhcange, const QString& symbol, const QString& type, quint64 id);
    quint64 getMoneyID(const TradingCatCommon::StockExchangeID& stockExchangeID, const TradingCatCommon::KLineID& klineID) const;

    using QueueMessages = std::queue<QString>; ///< Очередь запросов к БД
    using PQueueMessages = std::unique_ptr<QueueMessages>;///< Указатель на очередь запросов к БД

    /*!
        Сохраянет очередь запросов к БД в БД
        @param db ссылка на БД
        @param queueMessages - очередь запросов
        @return nullopt в случае успеха или строку с описанием ошибки
    */
    static std::optional<QString> saveToDB(QSqlDatabase& db/*, PQueueMessages insertQueueMessages*/, PQueueMessages updateQueueMessages);

    /*!
        Проверяет результаты сохранения очереди сообщений в БД
    */
    void checkResult();

    quint64 loadIDFromDB(const StockExchangeID &stockExcangeID, const KLineID& klineID);

private:
    using MoneyMap = QHash<TradingCatCommon::KLineID, quint64>;
    using StockExchangeMap = QHash<TradingCatCommon::StockExchangeID, MoneyMap>;

private:
    const Common::DBConnectionInfo _dbConnectionInfo;
    StockExchangeMap _klineTables;
    QSet<QString> _tablesList;

    QSqlDatabase _db;   ///< Сервисное подключение к БД
    QSqlDatabase _dbSaver;///< Полдключение к БД для сохранение свечей

    PQueueMessages _updateQuery;

    std::future<std::optional<QString>> _saveResult;  ///< результат хранения очереди сообщений

    bool _isStarted = false;

};

} //namespace TraidingCatBot
