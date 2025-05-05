#pragma once

//STL
#include <memory>
#include <list>

//Qt
#include <QDateTime>

//My
#include <Common/common.h>

#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     контейнер данных TradingDataConteiner class
///
class KLinesHistoryContainer
{
public:
    struct HistoryData
    {
        qint64 id = QDateTime::currentDateTime().toMSecsSinceEpoch(); //Может дублироваться
        TradingCatCommon::StockExchangeID stockExchangeId;
        TradingCatCommon::PKLine kline;

        HistoryData(const TradingCatCommon::StockExchangeID& astockExchangeId,
                    const TradingCatCommon::PKLine& akline)
            : stockExchangeId(astockExchangeId)
            , kline(akline)
        {
        }
    };

    using PHistoryData = std::shared_ptr<HistoryData>;
    using HistoryDataList = std::list<PHistoryData>;

public:
    KLinesHistoryContainer();

    /*!
        Деструктор
    */
    ~KLinesHistoryContainer();

    void addKLines(const TradingCatCommon::StockExchangeID& stockExchangeID, const TradingCatCommon::PKLinesList& klines);

    HistoryDataList getNewKLine(qint64 lastGetId, qint64 maxCount, const TradingCatCommon::KLineTypes& types) const;

private:
    Q_DISABLE_COPY_MOVE(KLinesHistoryContainer);

    void addHistoryKLine(const TradingCatCommon::StockExchangeID& stockExchangeID,
                         const PKLine& kline);

private:
    HistoryDataList _history;

};

} //namespace TradingCatCommon
