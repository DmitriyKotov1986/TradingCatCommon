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
