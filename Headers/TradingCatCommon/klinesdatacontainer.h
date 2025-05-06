#pragma once

//STL
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <optional>

//Qt
#include <QDateTime>
#include <QMutex>

//My
#include <Common/common.h>

#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     контейнер данных TradingDataConteiner class
///
class KLinesDataContainer final
{
private:
    struct KLineDateTimeReverseCmp
    {
        bool operator()(const qint64 dateTime1, const qint64 dateTime2) const noexcept
        {
            return dateTime1 > dateTime2;
        }
        size_t operator()(const qint64 dateTime) const noexcept
        {
            return dateTime;
        }
    };

public:
    using KLineMap = std::map<qint64, TradingCatCommon::PKLine, KLineDateTimeReverseCmp>;  // key dateTime msec from Epoch

public:
    /*!
        Конструктор. Создает пустой контейнер
    */
    KLinesDataContainer(const TradingCatCommon::StockExchangesIDList& stockExcangesIdList);

    /*!
        Деструктор
    */
    ~KLinesDataContainer() = default;

    /*!
        Доавляет список свечей в контейнер
        @param stockExchangeID - ИД биржи
        @param klines - список свечей
    */
    void addKLines(const TradingCatCommon::StockExchangeID& stockExchangeID, const TradingCatCommon::PKLinesList& klines);

    TradingCatCommon::PKLinesList getKLinesOnDate(const TradingCatCommon::StockExchangeID& stockExchangeID,
                                                 const TradingCatCommon::KLineID& klineID,
                                                 const qint64 start,
                                                 const qint64 end) const;

    const TradingCatCommon::StockExchangesIDList& getStockExcangeList() const noexcept;
    const TradingCatCommon::KLinesIDList& getKLineList(const TradingCatCommon::StockExchangeID& stockExchangeID) const noexcept;

    qsizetype moneyCount() const noexcept;

private:
    KLinesDataContainer() = delete;
    Q_DISABLE_COPY_MOVE(KLinesDataContainer);

private:
    const TradingCatCommon::StockExchangesIDList& _stockExchangesIdList;

    struct KLinesData
    {
        mutable QMutex mutex;
        std::unordered_map<TradingCatCommon::KLineID, KLineMap> klines;

        KLinesData() = default;
        KLinesData(KLinesData&& other)
        {
            std::swap(klines, other.klines);
        }
        KLinesData(const KLinesData& other) = delete;
        KLinesData& operator=(const KLinesData&) = delete;
        KLinesData& operator=(KLinesData&&) = delete;
    };
    std::unordered_map<size_t, KLinesData> _klinesData;

    struct StockExchangeData
    {
        std::unordered_map<TradingCatCommon::StockExchangeID, TradingCatCommon::KLinesIDList> data;
        mutable QMutex mutex;
    };
    StockExchangeData _stockExchangeData;

    struct MoneyCount
    {
        qsizetype count = 0;
        mutable QMutex mutex;
    };

    MoneyCount _moneyCount;
};

} //namespace TradingCatCommon
