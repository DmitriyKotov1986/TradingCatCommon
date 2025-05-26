#pragma once

//Qt
#include <QObject>

//My
#include <Common/common.h>
#include <Common/tdbloger.h>

#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"

#ifndef QT_NO_SSL

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     The IStockExchange class - интерфейсный класс биржи
///
class IStockExchange
    : public QObject
{
    Q_OBJECT

public:
    /*!
        Конструктор. Планируется использовать только этот конструктор
        @param id - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param parent - указатель на родительский класс
    */
    explicit IStockExchange(const TradingCatCommon::StockExchangeID& stockExchangeId, QObject* parent = nullptr);

    /*!
        Деструктор
    */
    ~IStockExchange() override = default;

    /*!
        Возвращает ИД биржи
        @return ИД биржи
     */
    const TradingCatCommon::StockExchangeID& id() const;

public slots:
    /*!
        Начало работы класса.
    */
    virtual void start();

    /*!
        Завершение работы класса
    */
    virtual void stop();

signals:
    /*!
        Испускается при получении новых свечей
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param klines - список новых свечей
    */
    void getKLines(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klines);

    /*!
        Возвращает список свечей поддерживаемых биржей
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param klinesId - список свечей поддерживаемых биржей
     */
    void getKLinesID(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesIDList& klinesId);

    /*!
        Сигнал генерируется если в процессе работы сервера произошла фатальная ошибка
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param errorCode - код ошибки
        @param errorString - текстовое описание ошибки
    */
    void errorOccurred(const TradingCatCommon::StockExchangeID& stockExchangeId, Common::EXIT_CODE errorCode, const QString& errorString);

    /*!
        Сообщение логеру
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsg(const TradingCatCommon::StockExchangeID& stockExchangeId, Common::TDBLoger::MSG_CODE category, const QString& msg);

    /*!
        Сингал генерируется после остановки работы класса
    */
    void finished();

private:
    // Удаляем неиспользуемые конструкторы
    IStockExchange() = delete;
    Q_DISABLE_COPY_MOVE(IStockExchange);

private:
    const TradingCatCommon::StockExchangeID _stockExchangeId;    ///< ИД Биржи

};

} //namespace TradingCatCommon

#endif
