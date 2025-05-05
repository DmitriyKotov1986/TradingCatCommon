///STL
#include <vector>
#include <algorithm>

#include "TradingCatCommon/natr.h"

using namespace TradingCatCommon;

static const quint64 MIN_SIZE_FOR_CALCULATE = 10u; ///< Минимальное колиесов свечей для расчета НАТРа
static const qint64 MAX_SAVE_NATR = 1000u;  ///<Количество интервалов свечи, когда расчитанный НАТР являться актуальным

Natr::Natr(const TradingCatCommon::PKLinesList &klines)
{
    calculate(klines);
}

void Natr::clear()
{
    _close.reset();
    _volume.reset();
}

void Natr::calculate(const TradingCatCommon::PKLinesList &klines)
{
    if (klines->size() <= MIN_SIZE_FOR_CALCULATE)
    {
        return;
    }

    if (_klineID.isEmpty())
    {
        _klineID = (*klines->begin())->id;
    }
    else
    {
        Q_ASSERT(_klineID == (*klines->begin())->id);
    }

    for (const auto& kline: *klines)
    {
        _old = std::max(_old, kline->closeTime);
    }

    //Delta
    {       
        std::vector<double> close;
        close.resize(klines->size());
        std::transform(klines->begin(), klines->end(), close.begin(),
            [](const auto& kline)
            {
                return kline->close;
            });

        std::sort(close.begin(), close.end());

        const auto ignoreCount = close.size() / 10;

        const auto closeSumm = std::accumulate(std::next(close.begin(), ignoreCount), std::prev(close.end(), ignoreCount), 0.0f);

        _close = closeSumm / (close.size() - (ignoreCount * 2));
    }

    //Volume
    {
        std::vector<double> volume;
        volume.reserve(klines->size());

        for (const auto& kline: *klines)
        {
            if (kline->volume > 10)
            {
                volume.push_back(kline->volume);
            }
        }

        if (volume.size() > MIN_SIZE_FOR_CALCULATE)
        {
            std::sort(volume.begin(), volume.end());

            const auto ignoreCount = volume.size() / 10;

            const auto volumeSumm = std::accumulate(std::next(volume.begin(), ignoreCount), std::prev(volume.end(), ignoreCount), 0.0f);

            _volume = volumeSumm / (volume.size() - (ignoreCount * 2));
        }
    }

    qDebug() << QString("KLine ID %1 update natr value successully. New value: close price: %2 volume: %3")
                    .arg((*klines->begin())->id.toString())
                    .arg(_close.value_or(0))
                    .arg(_volume.value_or(0));
}

const QDateTime &Natr::oldNatr() const noexcept
{
    return _old;
}

std::optional<double> Natr::close() noexcept
{
    if (_close.has_value() && _old.msecsTo(QDateTime::currentDateTime()) > static_cast<qint64>(_klineID.type) * MAX_SAVE_NATR)
    {
        _close.reset();
    }

    return _close;
}

std::optional<double> Natr::volume() noexcept
{
    if (_volume.has_value() && _old.msecsTo(QDateTime::currentDateTime()) > static_cast<qint64>(_klineID.type) * MAX_SAVE_NATR)
    {
        _volume.reset();
    }

    return _volume;
}

///////////////////////////////////////////////////////////////////////////////
///     class Natrs
///

Natr& Natrs::getNatr(const StockExchangeID &stockExchangeId, const KLineID &klineId)
{
    Q_ASSERT(!stockExchangeId.isEmpty());
    Q_ASSERT(!klineId.isEmpty());

    auto it_natrsMap = _natrsMap.find(stockExchangeId);
    if (it_natrsMap == _natrsMap.end())
    {
        auto p_klineNatrsMap = std::make_unique<KLineNatrsMap>();
        auto p_natr = std::make_unique<Natr>();
        p_klineNatrsMap->emplace(klineId, std::move(p_natr));

        it_natrsMap =  _natrsMap.emplace(stockExchangeId, std::move(p_klineNatrsMap)).first;
    }

    auto& klineNatrsMap = it_natrsMap ->second;

    auto it_klineNatrsMap = klineNatrsMap->find(klineId);
    if (it_klineNatrsMap == klineNatrsMap->end())
    {
        auto p_natr = std::make_unique<Natr>();
        it_klineNatrsMap = klineNatrsMap->emplace(klineId, std::move(p_natr)).first;
    }

    return *it_klineNatrsMap->second;
}

void Natrs::calculateNatr(const StockExchangeID &stockExchangeId, const PKLinesList &klines)
{
    Q_ASSERT(!stockExchangeId.isEmpty());
    Q_ASSERT(!klines->empty());

    if (klines->size() <= MIN_SIZE_FOR_CALCULATE)
    {
        return;
    }

    auto& natr = getNatr(stockExchangeId, (*klines->begin())->id);
    natr.calculate(klines);
}

const QDateTime &Natrs::oldNatr(const StockExchangeID &stockExchangeId, const KLineID &klineId)
{
    return getNatr(stockExchangeId, klineId).oldNatr();
}
