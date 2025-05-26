//My
#include <Common/parser.h>
#include "TradingCatCommon/transmitdata.h"

#include "TradingCatCommon/blacklistfilterdata.h"

using namespace TradingCatCommon;
using namespace Common;

BlackListFilterData::BlackListFilterData(const BlackListFilterData &blackListFilterData)
    : _stockExchangeID(blackListFilterData._stockExchangeID.has_value() ? std::make_optional(blackListFilterData._stockExchangeID.value()) : std::nullopt)
    , _klineID(blackListFilterData._klineID.has_value() ? std::make_optional(blackListFilterData._klineID.value()) : std::nullopt)
    , _errorString(blackListFilterData._errorString)
{
}

BlackListFilterData &BlackListFilterData::operator=(const BlackListFilterData& blackListFilterData)
{
    _stockExchangeID = blackListFilterData._stockExchangeID.has_value() ? std::make_optional(blackListFilterData._stockExchangeID.value()) : std::nullopt;
    _klineID = blackListFilterData._klineID.has_value() ? std::make_optional(blackListFilterData._klineID.value()) : std::nullopt;
    _errorString = blackListFilterData._errorString;

    return *this;
}

BlackListFilterData::BlackListFilterData(BlackListFilterData &&blackListFilterData)
    : _stockExchangeID(std::move(blackListFilterData._stockExchangeID))
    , _klineID(std::move(blackListFilterData._klineID))
    , _errorString(std::move(blackListFilterData._errorString))
{
}

BlackListFilterData &BlackListFilterData::operator=(BlackListFilterData&& blackListFilterData)
{
    _stockExchangeID = std::move(blackListFilterData._stockExchangeID);
    _klineID = std::move(blackListFilterData._klineID);
    _errorString = std::move(blackListFilterData._errorString);

    return *this;
}

BlackListFilterData::BlackListFilterData(const QJsonObject &json)
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
            const auto klineId =  KLineIDJson(klineIdJson);
            if (klineId.isError())
            {
                throw ParseException(QString("Error parsing [ID]: %1").arg(klineId.errorString()));
            }
            _klineID = klineId.klineId();
        }
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();

        return;
    }
}

const std::optional<StockExchangeID>& BlackListFilterData::stockExchangeID() const noexcept
{
    return _stockExchangeID;
}

const std::optional<KLineID>& BlackListFilterData::klineID() const noexcept
{
    return _klineID;
}

void BlackListFilterData::setKLineID(const std::optional<KLineID> &klineId)
{
    _klineID = klineId;
}

void BlackListFilterData::setStockExchangeID(const std::optional<StockExchangeID> &stockExchangeId)
{
    Q_ASSERT(!stockExchangeId->isEmpty());

    _stockExchangeID = stockExchangeId;
}

bool BlackListFilterData::isError() const noexcept
{
    return !_errorString.isEmpty();
}

QString BlackListFilterData::errorString()
{
    const auto err = _errorString;
    _errorString.clear();

    return err;
}

void BlackListFilterData::clear()
{
    _stockExchangeID.reset();
    _klineID.reset();

    _errorString.clear();
}

bool BlackListFilterData::check() const noexcept
{
    return (!_stockExchangeID.has_value() || (_stockExchangeID.has_value() && !_stockExchangeID->isEmpty())) &&
           (!_klineID.has_value()    || (_klineID.has_value() && !_klineID->isEmpty()));

}

bool BlackListFilterData::isAllKLineID() const noexcept
{
    return !_klineID.has_value();
}

bool BlackListFilterData::isAllStockExchange() const noexcept
{
    return !_stockExchangeID.has_value();
}

QJsonObject BlackListFilterData::toJson() const
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

    return json;
}
