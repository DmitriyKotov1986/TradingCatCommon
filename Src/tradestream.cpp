#include "TradingCatCommon/tradestream.h"

using namespace TradingCatCommon;


TradeStreamData::TradeStreamData(const Symbol &symbol, qint64 time)
    : _symbol(symbol)
    , _time(time)
{
    Q_ASSERT(!_symbol.isEmpty());
    Q_ASSERT(time != 0);
}

qint64 TradeStreamData::time() const noexcept
{
    return _time;
}

const Symbol &TradeStreamData::symbol() const noexcept
{
    return _symbol;
}

const TradeStreamData::TradeStreamList &TradeStreamData::tradeStreamList() const noexcept
{
    return _tradeStreamList;
}
