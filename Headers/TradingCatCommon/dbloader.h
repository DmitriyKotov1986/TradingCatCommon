#pragma once

//Qt
#include <QObject>

//My
#include <Common/sql.h>
#include <Common/tdbloger.h>
#include <Common/common.h>

#include "TradingCatCommon/stockexchange.h"
#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/klinesdatacontainer.h"

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     The DBLoader class - класс создание и загрузки контейнеров с данными по свечам
///
class DBLoader final:
    public QObject
{
    Q_OBJECT
public:
    /*!
        Конструктор. Планируется использовать только этот конструтор
        @param dbConnectionInfo - параметры подключения к БД. Значение должно быть корректным
        @param parent - указатель на родительски класс
    */
    explicit DBLoader(const Common::DBConnectionInfo& dbConnectionInfo, QObject *parent = nullptr);

    /*!
        Деструктор
    */
    ~DBLoader();

    /*!
        Выполнят подключение к БД и готовность загрузки данных. В случае ошибки - генерируется сигнал errorOccurred(...)
     */
    void start();

    /*!
        Завершает работу
    */
    void stop();

    /*!
        Создает контейнер с данными свечей
        @return  контейнер с данными свечей
    */
    std::unique_ptr<TradingCatCommon::KLinesDataContainer> loadFromDB();

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

private:
    struct MoneyInfo
    {
        StockExchangeID stockExchangeID;
        KLineID klineID;
        QDateTime lastClose = QDateTime::currentDateTime();
    };

private:
    // Удаляем неиспользуемые конструкторы
    DBLoader() = delete;
    Q_DISABLE_COPY_MOVE(DBLoader);

    void loadMoneyMap();
    void addMoneyInfo(const QString& stockExhcange, const QString& symbol, const QString& type, const QDateTime& lastClose, quint64 id);
    const MoneyInfo& getMoneyInfo(quint64 id) const;

private:
    const Common::DBConnectionInfo& _dbConnectionInfo;
    QSqlDatabase _db;

    std::unordered_map<quint64, MoneyInfo> _moneyMap;

    bool _isStarted = false;
};

} // namespace TradingCatCommon
