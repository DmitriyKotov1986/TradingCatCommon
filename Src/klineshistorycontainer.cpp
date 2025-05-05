// STL
#include <algorithm>

// Qt
#include <QMutex>
#include <QMutexLocker>

#include "TradingCatCommon/klineshistorycontainer.h"

using namespace TradingCatCommon;

static const qint64 NEW_INTERVAL = 60 * 60 * 1000; // 1 hour

static QMutex historyDataMutex;

KLinesHistoryContainer::KLinesHistoryContainer()
{
}

KLinesHistoryContainer::~KLinesHistoryContainer()
{
}

KLinesHistoryContainer::HistoryDataList KLinesHistoryContainer::getNewKLine(qint64 lastGetId, qint64 maxCount, const TradingCatCommon::KLineTypes& types) const
{
    HistoryDataList result;

    QMutexLocker<QMutex> historyDataMutexLocker(&historyDataMutex);

    if (_history.empty())
    {
        return result;
    }

    if (lastGetId == 0)
    {
        result.push_back(*_history.rbegin());

        return result;
    }

    const auto it_start = std::upper_bound(_history.begin(), _history.end(), lastGetId,
                                           [](const auto& start, const auto& p_historydata)
                                           {
                                               return p_historydata->id > start;
                                           });

    if (it_start == _history.end())
    {
        return result;
    }

    const auto& lastId= _history.rbegin()->get()->id;
    for (auto it_history = it_start; it_history != _history.end() && it_history->get()->id != lastId && maxCount > 0; ++it_history)
    {
        if (types.contains(it_history->get()->kline->id.type))
        {
            result.push_back(*it_history);

            --maxCount;
        }
    }

    return result;
}

void KLinesHistoryContainer::addHistoryKLine(const StockExchangeID &stockExchangeID, const PKLine& kline)
{
    const auto oldestKLine = QDateTime::currentDateTime().addMSecs(-NEW_INTERVAL);

    if (kline->closeTime < oldestKLine)
    {
        return;
    }

    auto data = std::make_shared<HistoryData>(stockExchangeID, kline);

    QMutexLocker<QMutex> historyDataMutexLocker(&historyDataMutex);

    _history.emplace_back(std::move(data));

    if (QDateTime::fromMSecsSinceEpoch(_history.begin()->get()->id) < oldestKLine)
    {
        _history.pop_front();
    }
}

void KLinesHistoryContainer::addKLines(const StockExchangeID& stockExchangeID, const PKLinesList& klinesList)
{
    Q_ASSERT(!stockExchangeID.isEmpty());

    if (klinesList->empty())
    {
        return;
    }

    for (const auto& kline: *klinesList)
    {
        addHistoryKLine(stockExchangeID, kline);
    }
}
