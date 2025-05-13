#pragma once

//STL
#include <memory>

//Qt
#include <QObject>

//My
#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"
#include "TradingCatCommon/klinesdatacontainer.h"

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     The TradingData class - контейнер данных полученных с бирже свечей
///
class TradingData
    : public QObject
{
    Q_OBJECT

public:
    explicit TradingData(const TradingCatCommon::StockExchangesIDList& stockExcangesIdList, QObject* parent = nullptr);

    /*!
        Деструктор
    */
    ~TradingData();

    //for HTTP server
    /*!
        Возвращает список свечей за заданные период
        @param stockExchangeID  - ИД Биржи
        @param klineID - ИД свечи
        @param start - время начала периода (мсек epoch)
        @param end - время конца периода (мсек epoch)
        @return список свечей
    */
    TradingCatCommon::PKLinesList getKLinesOnDate(const TradingCatCommon::StockExchangeID &stockExchangeID,
                                                  const TradingCatCommon::KLineID& klineID,
                                                  const qint64 start,
                                                  const qint64 end) const;

    /*!
        Возвращает общее количество монет
        @return общее количество монет
    */
    qsizetype moneyCount() const noexcept;

    /*!
        Возвращает список поддерживаемых бирж
        @return список поддерживаемых бирж
    */
    const TradingCatCommon::StockExchangesIDList& stockExcangesIdList() const noexcept;

    /*!
        Возвращает список свечей поддерживаемых данной биржей
        @param stockExchangeID - ИД биржи
        @return список свечей  данной биржей
    */
    const TradingCatCommon::KLinesIDList& getKLineList(const TradingCatCommon::StockExchangeID& stockExchangeID) const;

signals:
    /*!
        Сигнал испускаеться когда класс завершил работу
    */
    void finished();

public slots:
    /*!
        Начало работы класса.
    */
    void start();

    /*!
        Завершение работы класса
    */
    void stop();

    /*!
        Сохраняет послупвшие от бирж свечи
        @param stockExchangeID - ИД биржи
        @param klines - список свечей
    */
    void addKLines(const TradingCatCommon::StockExchangeID& stockExchangeID, const TradingCatCommon::PKLinesList& klines);

private:
    // Удаляем неиспользуемые конструторы
    TradingData() = delete;
    Q_DISABLE_COPY_MOVE(TradingData);

private:
    const TradingCatCommon::StockExchangesIDList _stockExcangesIdList;  ///< Список используемых бирж
    std::unique_ptr<TradingCatCommon::KLinesDataContainer> _dataKLine;  ///< Данные

    bool _isStarted = false;    ///< Флаг успешного запуска

};

} // namespace TradingCatCommon
