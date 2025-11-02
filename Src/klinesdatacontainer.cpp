// STL
#include <algorithm>

// Qt
#include <QMutexLocker>
#include <QRegularExpression>

#include "TradingCatCommon/klinesdatacontainer.h"

using namespace TradingCatCommon;

static const qsizetype COUNT_SAVE_KLINES = 1000;

static size_t hash(const TradingCatCommon::StockExchangeID& stockExchangesId, const QString& addons)
{
    Q_ASSERT(!addons.isEmpty());

    static QRegularExpression regex(R"([a-zA-Z0-9])");

    return qHash(stockExchangesId, 0) + 127 * qHash(regex.match(addons).captured(0));
}

KLinesDataContainer::KLinesDataContainer(const TradingCatCommon::StockExchangesIDList& stockExcangesIdList)
    : _stockExchangesIdList(stockExcangesIdList)
{
    Q_ASSERT(!_stockExchangesIdList.empty());

    for (const auto& stockExcangeId : _stockExchangesIdList)
    {
        for (int ch = 'A'; ch <= 'Z'; ++ch)
        {
            _klinesData.emplace(hash(stockExcangeId, QChar(ch)), KLinesData());
        }
        for (int ch = 'a'; ch <= 'z'; ++ch)
        {
            _klinesData.emplace(hash(stockExcangeId, QChar(ch)), KLinesData());
        }
        for (int ch = '0'; ch <= '9'; ++ch)
        {
            _klinesData.emplace(hash(stockExcangeId, QChar(ch)), KLinesData());
        }
    }
}

PKLinesList KLinesDataContainer::getKLinesOnDate(const StockExchangeID &stockExchangeId, const KLineID &klineId, qint64 start, qint64 end) const
{
    Q_ASSERT(!stockExchangeId.isEmpty());
    Q_ASSERT(!klineId.isEmpty());
    Q_ASSERT(start < end);

    const auto moneyHash = hash(stockExchangeId, klineId.symbol.name);
    const auto& moneyData = _klinesData.at(moneyHash);

    auto result = std::make_shared<KLinesList>();

    QMutexLocker<QMutex> moneyDataLocker(&moneyData.mutex);

    const auto& klines = moneyData.klines;
    const auto it_klines = klines.find(klineId);
    if (it_klines == klines.end())
    {
        return result;
    }

    const auto& klineMap = it_klines->second;
    const auto it_klineMapStart = klineMap.lower_bound(end);
    const auto it_klineMapEnd = klineMap.upper_bound(start);

    result->resize(std::distance(it_klineMapStart, it_klineMapEnd));

    std::transform(it_klineMapStart, it_klineMapEnd, result->begin(),
                   [](const auto& p_kline)
                   {
                       return p_kline.second;
                   });

    return result;
}

const TradingCatCommon::StockExchangesIDList& KLinesDataContainer::getStockExcangeList() const noexcept
{
    return _stockExchangesIdList;
}

qsizetype KLinesDataContainer::moneyCount() const noexcept
{
    QMutexLocker<QMutex> moneyDataLocker(&_moneyCount.mutex);

    return _moneyCount.count;
}

void KLinesDataContainer::addKLines(const StockExchangeID& stockExchangeId, const PKLinesList& klines)
{
    Q_ASSERT(!stockExchangeId.isEmpty());
    Q_ASSERT(!klines->empty());

    KLinesList localKLinesList(*klines);

    //Сортируем данные, если они все еще не отсортированы
    localKLinesList.sort(
        [](const auto& kline1, const auto& kline2)
        {
            return kline1->closeTime < kline2->closeTime;
        });

    const auto klineId = klines->front()->id;
    const auto moneyHash = hash(stockExchangeId, klineId.symbol.name);
    auto& moneyData = _klinesData.at(moneyHash);
    auto& klinesData = moneyData.klines;
    auto it_klinesData = klinesData.find(klineId);

    QMutexLocker<QMutex> moneyDataLocker(&moneyData.mutex);

    if (it_klinesData == klinesData.end())
    {
        it_klinesData = klinesData.emplace(klineId, KLineMap()).first;

        {
            QMutexLocker<QMutex> moneyDataLocker(&_moneyCount.mutex);
            ++_moneyCount.count;
        }
    }
    auto& klineMap = it_klinesData->second;

#ifdef QT_DEBUG
    QStringList addedMissing;
#endif
    for (auto& localKline: localKLinesList)
    {
        const auto closeDateTime = localKline->closeTime;

        if (klineMap.empty())
        {
            klineMap.emplace(std::move(closeDateTime), std::move(localKline));

            continue;
        }

        auto& lastClose = klineMap.begin()->first;
        const auto interval = static_cast<qint64>(klineId.type);
        const auto deltaTime = closeDateTime - lastClose;

        Q_ASSERT(deltaTime != 0);

        if (deltaTime <= 0)
        {
            continue;
        }

        if (deltaTime > interval + 1)
        {
            auto& lastKLine = klineMap.begin()->second;

            //Расчитываем сколько свечей не хватает
            const quint64 stepCount = ((deltaTime + 1) / interval) - 1;

            //Добавляем недостающие свечи
            auto newCloseTime = lastClose;
            for (quint64 step = 0; step < stepCount; ++step)
            {
                const auto tmp_kline = std::make_shared<KLine>();
                tmp_kline->id = lastKLine->id;
                tmp_kline->open = lastKLine->open;
                tmp_kline->close = lastKLine->open;
                tmp_kline->high = lastKLine->open;
                tmp_kline->low = lastKLine->open;
                tmp_kline->volume = 0;
                tmp_kline->quoteAssetVolume = 0;
                tmp_kline->openTime = newCloseTime;
                newCloseTime = newCloseTime + interval;
                tmp_kline->closeTime = newCloseTime;

                auto tmpKLineCloseTime = tmp_kline->closeTime;

                klineMap.emplace(std::move(tmpKLineCloseTime), std::move(tmp_kline));
            }

#ifdef QT_DEBUG
            addedMissing.push_back(QString("(%1-%2)")
                                       .arg(QDateTime::fromMSecsSinceEpoch(lastClose).toString(Common::SIMPLY_DATETIME_FORMAT))
                                       .arg(QDateTime::fromMSecsSinceEpoch(closeDateTime).toString(Common::SIMPLY_DATETIME_FORMAT)));
#endif
        }

        klineMap.emplace(std::move(closeDateTime), std::move(localKline));
    }

#ifdef QT_DEBUG
    if (!addedMissing.isEmpty())
    {
        qWarning() << QString("StockExchange ID: %1 KLine ID: %2. Missing candlesticks with zero volume have been added. Between: %3")
            .arg(stockExchangeId.toString())
            .arg(klineId.toString())
            .arg(addedMissing.join(','));
    }
#endif

    if (klineMap.size() > COUNT_SAVE_KLINES)
    {
        klineMap.erase(std::next(klineMap.begin(), COUNT_SAVE_KLINES - 1), std::prev(klineMap.end()));
    }
}
