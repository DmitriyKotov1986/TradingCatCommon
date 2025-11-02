#pragma once

//STL
#include <memory>

//Qt
#include <QObject>

//My
#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/symbol.h"
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
                                                  qint64 start,
                                                  qint64 end) const;

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
        Возвращает список свечей поддерживаемых данной биржей. Если ИД биржи неихвестно - метод вернет указатель на пустой список
        @param stockExchangeID - ИД биржи
        @return список свечей данной биржей. Гарантируется что метод вернет указатель не равный nullptr
    */
    const TradingCatCommon::PKLinesIDList& getKLinesIDList(const TradingCatCommon::StockExchangeID& stockExchangeID) const;

    /*!
        Возвращает список бирж на которых залистина даная монета
        @param symbol - название монеты
        @return список бирж
    */
    const TradingCatCommon::StockExchangesIDList& moneyListing(const TradingCatCommon::Symbol& symbol) const noexcept;

signals:
    /*!
        Сигнал испускаеться когда класс завершил работу
    */
    void finished();

    /*!
        Сигнал испускаеться когда класс начал работу и получены данные о свечах от всех бирж
    */
    void started();

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
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param klines - список свечей
    */
    void addKLines(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klines);

    /*!
        Возвращает список свечей поддерживаемых биржей
        @param stockExchangeId - ИД биржи. Гарантируется что ИД биржи валидно и не пустое
        @param klinesId - список свечей поддерживаемых биржей
     */
    void getKLinesID(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesIDList& klinesId);

private:
    // Удаляем неиспользуемые конструторы
    TradingData() = delete;
    Q_DISABLE_COPY_MOVE(TradingData);

private:
    const TradingCatCommon::StockExchangesIDList _stockExcangesIdList;  ///< Список используемых бирж
    std::unordered_map<TradingCatCommon::StockExchangeID, TradingCatCommon::PKLinesIDList> _klinesIdList;  ///< Список свичей по биржам
    std::unique_ptr<TradingCatCommon::KLinesDataContainer> _dataKLine;  ///< Данные
    std::unordered_map<TradingCatCommon::Symbol, TradingCatCommon::StockExchangesIDList> _moneyListing; ///< Список бирж по монетам (на каких биржах монета залистина)

    bool _isStarted = false;    ///< Флаг успешного запуска
    bool _isSendStarted = false;

};

} // namespace TradingCatCommon
