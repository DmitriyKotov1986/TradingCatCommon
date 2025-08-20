#include <TradingCatCommon/orderbook.h>

const TradingCatCommon::OrderBook::TAsksList &TradingCatCommon::OrderBook::asks() const noexcept
{
    return _asks;
}

const TradingCatCommon::OrderBook::TBidsList &TradingCatCommon::OrderBook::bids() const noexcept
{
    return _bids;
}

TradingCatCommon::OrderBook::TBestPrice TradingCatCommon::OrderBook::bestPrice() const
{
    TBestPrice result;
    if (!_asks.empty())
    {
        result.ask = *_asks.begin();
    }
    if (!_bids.empty())
    {
        result.bid = *_bids.begin();
    }

    return result;
}

TradingCatCommon::OrderBook::OrderBook(const TradingCatCommon::Symbol& symbol, qint64 version)
    : _version(version)
    , _symbol(symbol)
{
    Q_ASSERT(_version != 0);
    Q_ASSERT(!_symbol.isEmpty());
}

qint64 TradingCatCommon::OrderBook::version() const noexcept
{
    Q_ASSERT(_version != 0);

    return _version;
}

const TradingCatCommon::Symbol &TradingCatCommon::OrderBook::symbol() const noexcept
{
    Q_ASSERT(!_symbol.isEmpty());

    return _symbol;
}
