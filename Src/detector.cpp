//STL
#include <cfloat>

#include "TradingCatCommon/detector.h"

using namespace TradingCatCommon;

///////////////////////////////////////////////////////////////////////////////
///     class Detector
///
Detector::Detector(const TradingCatCommon::TradingData& tradingData, QObject *parent /* = nullptr*/)
    : QObject{parent}
    , _tradingData(tradingData)
{
    qRegisterMetaType<TradingCatCommon::Filter>("TradingCatCommon::UserConfig");
    qRegisterMetaType<TradingCatCommon::Detector::PKLineDetectData>("TradingCatCommon::Detector::PKLineDetectData");
}

void Detector::start()
{

}

void Detector::stop()
{
    emit finished();
}

void Detector::userOnline(qint64 sessionId, const TradingCatCommon::UserConfig& config)
{
    auto it_filters = _filters.find(sessionId);
    if (it_filters == _filters.end())
    {
        _filters.emplace(sessionId, config.filter());
    }
    else
    {
        it_filters->second = config.filter();
    }
}

void Detector::userOffline(qint64 sessionId)
{
    _filters.erase(sessionId);
}

void Detector::addKLines(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klines)
{
    Q_ASSERT(!stockExchangeId.isEmpty());
    Q_ASSERT(!klines->empty());

    if (klines->front()->id.type != KLineType::MIN1)
    {
        return;
    }

    const auto currDateTime = QDateTime::currentDateTime();

    for (const auto& kline: *klines)
    {
        Q_ASSERT(!kline->id.isEmpty());

        if (kline->closeTime.secsTo(currDateTime) > 60 * 20)
        {
            continue;
        }

        const auto it_alreadyDetect = _alreadyDetectKLine.find(kline->id);
        if (it_alreadyDetect != _alreadyDetectKLine.end())
        {
            if (it_alreadyDetect->second.secsTo(currDateTime) > 60 * 10)
            {
                _alreadyDetectKLine.erase(it_alreadyDetect);
            }
            else
            {
                continue;
            }
        }

        for (const auto& [sessionId, filter]: _filters)
        {
            const auto& filtersDataList = filter.klineFilter();

            const auto it_filterData = std::find_if(filtersDataList.begin(), filtersDataList.end(),
                [&stockExchangeId, &kline](const auto& filterData)
                {
                    const auto& filterStockExchangeID = filterData.stockExchangeID();
                    if (!filterStockExchangeID.has_value() || (filterStockExchangeID.value() == stockExchangeId))
                    {
                        const auto& filterKLineID = filterData.klineID();
                        if (!filterKLineID.has_value() || (filterKLineID.value() == kline->id))
                        {
                            return true;
                        }
                    }

                    return false;
                });

            if (it_filterData == filtersDataList.end())
            {
                continue;
            }

            const auto& filterData = *it_filterData;
            QString msg;
            Filter::FilterTypes detectType(Filter::FilterType::UNDETECT);
            const auto delta = kline->deltaKLine();
            // if (stockExchangeId.name == "OKX")
            // {
            //      qInfo() << "OKX delta/volume" << kline->id.symbol << kline->deltaKLine() << kline->volumeKLine();
            // }
            const auto filterDelta = filterData.delta().value_or(DBL_MAX);
            if ((delta > filterDelta))
            {
                qDebug() << "Detect DELTA. ID: " << kline->id.toString() << " KLine: " << delta << " Filter: " << filterDelta;

                detectType.setFlag(Filter::UNDETECT, false);
                detectType.setFlag(Filter::DELTA);

                msg += QString(", %1:%2").arg(Filter::filterTypeToString(Filter::DELTA)).arg(delta, 0, 'f');
            }

            const auto volume = kline->volumeKLine();
            const auto& filterVolume = filterData.volume().value_or(DBL_MAX);
            if (volume > filterVolume)
            {
                qDebug() << "Detect VOLUME. ID: " << kline->id.toString() << " KLine: " << volume << " Filter: " << filterVolume;

                detectType.setFlag(Filter::UNDETECT, false);
                detectType.setFlag(Filter::VOLUME);

                msg += QString(", %1:%2(%3 slots)").arg(Filter::filterTypeToString(Filter::VOLUME)).arg(volume, 0, 'f').arg(kline->volume, 0, 'f', 0);
            }

            if (detectType.testFlag(Filter::DELTA) && detectType.testFlag(Filter::VOLUME))
            {
                msg = QString("(%1) %2").arg(kline->closeTime.toString("hh:mm")).arg(msg.remove(0, 2));

                const auto& klineIdHistory = kline->id;
                auto klineIdReviewHistory = klineIdHistory;
                klineIdReviewHistory.type = TradingCatCommon::KLineType::MIN5;

                const auto history = _tradingData.getKLinesOnDate(stockExchangeId, kline->id, currDateTime.addMSecs(-static_cast<quint64>(klineIdHistory.type) * (KLINES_COUNT_HISTORY + 1)), currDateTime);
                if (history->size() < KLINES_COUNT_HISTORY)
                {
                    qDebug() << "History size to less : " << history->size();

                    continue;
                }

                const auto reviewHistory = _tradingData.getKLinesOnDate(stockExchangeId, klineIdReviewHistory, currDateTime.addMSecs(-static_cast<quint64>(klineIdReviewHistory.type) * (KLINES_COUNT_HISTORY + 1)), currDateTime);
                if (reviewHistory->size() < KLINES_COUNT_HISTORY)
                {
                    qDebug() << "Review history size to less" << reviewHistory->size();

                    continue;
                }

                const auto detectData = std::make_shared<KLineDetectData>();

                detectData->stockExchangeId = stockExchangeId;
                detectData->filterActivate = detectType;
                detectData->history = std::move(history);
                detectData->reviewHistory = std::move(reviewHistory);
                detectData->delta = delta;
                detectData->volume = volume;
                detectData->msg = msg;

                emit klineDetect(sessionId, detectData);

                _alreadyDetectKLine.emplace(kline->id, currDateTime);
            }
            else
            {
                qDebug() << "No detect. ID: " << kline->id.toString();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
///     KLinesDetectedList
///
void Detector::KLinesDetectedList::clear()
{
    detected.clear();
    isFull = false;
}
