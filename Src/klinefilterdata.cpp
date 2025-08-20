//My
#include <Common/parser.h>
#include "TradingCatCommon/transmitdata.h"

#include "TradingCatCommon/klinefilterdata.h"

using namespace TradingCatCommon;
using namespace Common;

static const float EPSILON_POSITIVE = std::numeric_limits<float>::epsilon();               ///< минимальное пороговое значнеие цены и объема

KLineFilterData::KLineFilterData(const KLineFilterData &klineFilterData)
    : _stockExchangeID(klineFilterData._stockExchangeID.has_value() ? std::make_optional(klineFilterData._stockExchangeID.value()) : std::nullopt)
    , _klineID(klineFilterData._klineID.has_value() ? std::make_optional(klineFilterData._klineID.value()) : std::nullopt)
    , _delta(klineFilterData._delta.has_value() ? std::make_optional(klineFilterData._delta.value()) : std::nullopt)
    , _volume(klineFilterData._volume.has_value() ? std::make_optional(klineFilterData._volume.value()) : std::nullopt)
    , _errorString(klineFilterData._errorString)
{
}

KLineFilterData &KLineFilterData::operator=(const KLineFilterData& klineFilterData)
{
    _stockExchangeID = klineFilterData._stockExchangeID.has_value() ? std::make_optional(klineFilterData._stockExchangeID.value()) : std::nullopt;
    _klineID = klineFilterData._klineID.has_value() ? std::make_optional(klineFilterData._klineID.value()) : std::nullopt;
    _delta = klineFilterData._delta.has_value() ? std::make_optional(klineFilterData._delta.value()) : std::nullopt;
    _volume = klineFilterData._volume.has_value() ? std::make_optional(klineFilterData._volume.value()) : std::nullopt;
    _errorString = klineFilterData._errorString;

    return *this;
}

KLineFilterData::KLineFilterData(KLineFilterData &&klineFilterData)
    : _stockExchangeID(std::move(klineFilterData._stockExchangeID))
    , _klineID(std::move(klineFilterData._klineID))
    , _delta(std::move(klineFilterData._delta))
    , _volume(std::move(klineFilterData._volume))
    , _errorString(std::move(klineFilterData._errorString))
{
}

KLineFilterData &KLineFilterData::operator=(KLineFilterData&& klineFilterData)
{
    _stockExchangeID = std::move(klineFilterData._stockExchangeID);
    _klineID = std::move(klineFilterData._klineID);
    _delta = std::move(klineFilterData._delta);
    _volume = std::move(klineFilterData._volume);
    _errorString = std::move(klineFilterData._errorString);

    return *this;
}

KLineFilterData::KLineFilterData(const QJsonObject &json)
{
    try
    {
        if (json.contains("StockExchange"))
        {
            const auto stockExchangeIDJson = JSONReadMapToMap(json, "StockExchange", "StockExchange");
            const auto stockExchangeId = StockExchangeIDJson(stockExchangeIDJson);
            if (stockExchangeId.isError())
            {
                throw ParseException(QString("Error parsing [StockExchange]: %1").arg(stockExchangeId.errorString()));
            }

            _stockExchangeID = stockExchangeId.stockExchangeId();
        }
        if (json.contains("ID"))
        {
            const auto klineIdJson = JSONReadMapToMap(json, "ID", "ID");
            const auto klineId =  KLineIDJson(json);
            if (klineId.isError())
            {
                throw ParseException(QString("Error parsing [ID]: %1").arg(klineId.errorString()));
            }
            _klineID =klineId.klineId();

        }
        if (json.contains("Delta"))
        {
            _delta = JSONReadMapNumber<float>(json, "Delta", QString("Delta"), 0.0f);
        }
        if (json.contains("Volume"))
        {
            _volume = JSONReadMapNumber<float>(json, "Volume", QString("Volume"), 0.0f);
        }
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();

        return;
    }
}

const std::optional<StockExchangeID>&KLineFilterData::stockExchangeID() const noexcept
{
    return _stockExchangeID;
}

const std::optional<KLineID> &KLineFilterData::klineID() const noexcept
{
    return _klineID;
}

const std::optional<float> &KLineFilterData::delta() const noexcept
{
    return _delta;
}

const std::optional<float> &KLineFilterData::volume() const noexcept
{
    return _volume;
}

void KLineFilterData::setDelta(const std::optional<float> &delta)
{
    Q_ASSERT(delta.has_value() && delta >= (MinDelta - EPSILON_POSITIVE) && delta <= (MaxDelta + EPSILON_POSITIVE));

    _delta = delta;
}

void KLineFilterData::setVolume(const std::optional<float> &volume)
{
    Q_ASSERT(volume.has_value() && volume >= (MinVolume - EPSILON_POSITIVE) && volume <= (MaxVolume + EPSILON_POSITIVE));

    _volume = volume;
}

void KLineFilterData::setKLlineID(const std::optional<KLineID> &klineId)
{
    Q_ASSERT(klineId->isEmpty());

    _klineID = klineId;
}

void KLineFilterData::setStockExchangeID(const std::optional<StockExchangeID> &stockExchangeId)
{
    Q_ASSERT(!stockExchangeId->isEmpty());

    _stockExchangeID = stockExchangeId;
}

bool KLineFilterData::isError() const noexcept
{
    return !_errorString.isEmpty();
}

QString KLineFilterData::errorString()
{
    const auto err = _errorString;
    _errorString.clear();

    return err;
}

void KLineFilterData::clear()
{
    _stockExchangeID.reset();
    _klineID.reset();
    _delta.reset();
    _volume.reset();

    _errorString.clear();
}

bool KLineFilterData::check() const noexcept
{
    return (!_stockExchangeID.has_value() || (_stockExchangeID.has_value() && !_stockExchangeID->isEmpty())) &&
           (!_klineID.has_value()    || (_klineID.has_value() && !_klineID->isEmpty())) &&
           (!_delta.has_value()      || (_delta.has_value() && _delta.value() >= (MinDelta - EPSILON_POSITIVE) && _delta.value() <= (MaxDelta + EPSILON_POSITIVE))) &&
           (!_volume.has_value()     || (_volume.has_value() && _volume.value() >= (MinVolume - EPSILON_POSITIVE) && _volume.value() <= (MaxVolume + EPSILON_POSITIVE)));

}

bool KLineFilterData::isAllKLineID() const noexcept
{
    return !_klineID.has_value();
}

bool KLineFilterData::isAllStockExchange() const noexcept
{
    return !_stockExchangeID.has_value();
}

QJsonObject KLineFilterData::toJson() const
{
    QJsonObject json;

    if (_stockExchangeID.has_value())
    {
        json.insert("StockExchange", StockExchangeIDJson(_stockExchangeID.value()).toJson());
    }
    if (_klineID.has_value())
    {
        json.insert("ID", KLineIDJson(_klineID.value()).toJson());
    }
    if (_delta.has_value())
    {
        json.insert("Delta", _delta.value());
    }
    if (_volume.has_value())
    {
        json.insert("Volume", _volume.value());
    }

    return json;
}
