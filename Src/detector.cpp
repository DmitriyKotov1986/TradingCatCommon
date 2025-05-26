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

    const auto& klineId = klines->front()->id;

    if (klineId.type != KLineType::MIN1)
    {
        return;
    }

    const auto currDateTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    for (const auto& [sessionId, filter]: _filters)
    {
        const auto& blackListDataList = filter.blackList();
        bool isBlackList = false;

        for (const auto& blackListData: blackListDataList)
        {
            if (!blackListData.stockExchangeID().has_value() || stockExchangeId == blackListData.stockExchangeID().value())
            {
                if (!blackListData.klineID().has_value() || klineId == blackListData.klineID().value())
                {
                    isBlackList = true;
                    break;
                }
            }
        }
        if (isBlackList)
        {
 //           qDebug() << klineId.toString() << "is black list. Skip";

            continue;
        }

        for (const auto& kline: *klines)
        {
            Q_ASSERT(!kline->id.isEmpty());

            if ((currDateTime - kline->closeTime) > 60 * 20 * 1000) // msec
            {
                continue;
            }

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
            // if (stockExchangeId.name == "KUCOIN_FUTURES")
            // {
            //      qInfo() << "KUCOIN_FUTURES delta/volume" << kline->id.symbol << kline->deltaKLine() << kline->volumeKLine();
            // }
            const auto filterDelta = filterData.delta().value_or(DBL_MAX);
            if ((delta > filterDelta))
            {
                //qDebug() << "Detect DELTA. ID: " << kline->id.toString() << " KLine: " << delta << " Filter: " << filterDelta;

                detectType.setFlag(Filter::UNDETECT, false);
                detectType.setFlag(Filter::DELTA);

                msg += QString(", %1:%2").arg(Filter::filterTypeToString(Filter::DELTA)).arg(delta, 0, 'f');
            }

            const auto volume = kline->volumeKLine();
            const auto& filterVolume = filterData.volume().value_or(DBL_MAX);
            if (volume > filterVolume)
            {
                //qDebug() << "Detect VOLUME. ID: " << kline->id.toString() << " KLine: " << volume << " Filter: " << filterVolume;

                detectType.setFlag(Filter::UNDETECT, false);
                detectType.setFlag(Filter::VOLUME);

                msg += QString(", %1:%2(%3 slots)")
                           .arg(Filter::filterTypeToString(Filter::VOLUME))
                           .arg(volume, 0, 'f')
                           .arg(kline->volume, 0, 'f', 0);
            }

            if (detectType.testFlag(Filter::DELTA) && detectType.testFlag(Filter::VOLUME))
            {
                msg = QString("%1->%2 (%3) High:%4 Open:%5 Close:%6 Low:%7 Volume:%8 %9")
                          .arg(stockExchangeId.toString())
                          .arg(kline->id.toString())
                          .arg(QDateTime::fromMSecsSinceEpoch(kline->closeTime).toString("hh:mm"))
                          .arg(kline->high)
                          .arg(kline->open)
                          .arg(kline->close)
                          .arg(kline->low)
                          .arg(kline->volume)
                          .arg(msg.remove(0, 2));

                const auto& klineIdHistory = kline->id;
                auto klineIdReviewHistory = klineIdHistory;
                klineIdReviewHistory.type = TradingCatCommon::KLineType::MIN5;

                const auto history = _tradingData.getKLinesOnDate(stockExchangeId,
                                                                  kline->id,
                                                                  currDateTime - (static_cast<qint64>(klineIdHistory.type) * (KLINES_COUNT_HISTORY)) - static_cast<qint64>(klineIdHistory.type) / 2,
                                                                  currDateTime + static_cast<qint64>(klineIdHistory.type) / 2);
                if (history->empty())
                {
                    //qDebug() << "History size to less : " << history->size();

                    continue;
                }

                const auto reviewHistory = _tradingData.getKLinesOnDate(stockExchangeId,
                                                                        klineIdReviewHistory,
                                                                        currDateTime - (static_cast<qint64>(klineIdReviewHistory.type) * (KLINES_COUNT_HISTORY)) - static_cast<qint64>(klineIdReviewHistory.type) / 2,
                                                                        currDateTime + static_cast<qint64>(klineIdReviewHistory.type) / 2);
                if (reviewHistory->empty())
                {
                    //qDebug() << "Review history size to less" << reviewHistory->size();

                    continue;
                }

                const auto detectData = std::make_shared<KLineDetectData>();

                detectData->stockExchangeId = stockExchangeId;
                detectData->filterActivate = detectType;
                detectData->history = std::move(history);
                detectData->reviewHistory = std::move(reviewHistory);
                detectData->delta = delta;
                detectData->volume = volume;
                detectData->msg =std::move(msg);

                emit klineDetect(sessionId, detectData);
            }
            else
            {
                //qDebug() << "No detect. ID: " << kline->id.toString();
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
