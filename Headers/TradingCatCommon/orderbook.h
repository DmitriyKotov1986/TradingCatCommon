#pragma once

//STL
#include <map>
#include <utility>
#include <limits>
#include <memory>

//Qt
#include <QMetaType>

namespace TradingCatCommon
{

using Order = std::pair<float, float>;
using POrder = std::shared_ptr<Order>;

class OrderBook final
{
private:
    struct OrderReverseCmp
    {
        bool operator()(const float price1, const float price2) const noexcept
        {
            return price1 > price2;
        }
    };
public:
    using TAsksList = std::map<float, float,  OrderReverseCmp>;
    using TBidsList = std::map<float, float>;

    struct TBestPrice
    {
        Order ask = Order{0.0f, 0.0f};
        Order bid = Order{0.0f, 0.0f};
    };

public:
    explicit OrderBook(qint64 version);
    ~OrderBook() = default;

    const TAsksList& asks() const noexcept;
    const TBidsList& bids() const noexcept;

    TBestPrice bestPrice() const;

    template <class TTAsksList>
    void setAsks(TTAsksList&& asks);

    template <class TTBidsList>
    void setBids(TBidsList&& bids);

    template <class TOrder>
    void addDiffAks(TOrder order);

    template <class TOrder>
    void addDiffBid(TOrder order);

private:
    OrderBook() = delete;

private:
    TAsksList _asks;
    TBidsList _bids;

    qint64 version = 0;

};

template<class TOrder>
inline void OrderBook::addDiffBid(TOrder order)
{
    const auto it_bids = _bids.find(order.second);
    if (order.second < std::numeric_limits<decltype(order.second)>::epsilon())
    {
        if (it_bids != _bids.end())
        {
            _asks.erase(it_bids);
        }
    }
    else
    {
        if (it_bids != _asks.end())
        {
            it_bids->second = std::forward<Order>(order);
        }
        else
        {
            _bids.emplace(std::forward<Order>(order));
        }
    }
}

template<class TOrder>
inline void OrderBook::addDiffAks(TOrder order)
{
    const auto it_asks = _asks.find(order.second);
    if (order.second < std::numeric_limits<decltype(order.second)>::epsilon())
    {
        if (it_asks != _asks.end())
        {
            _asks.erase(it_asks);
        }
    }
    else
    {
        if (it_asks != _asks.end())
        {
            it_asks->second = std::forward<Order>(order);
        }
        else
        {
            _asks.emplace(std::forward<Order>(order));
        }
    }
}

template<class TTBidsList>
inline void OrderBook::setBids(TBidsList &&bids)
{
    _bids = std::forward<TBidsList>(bids);
}

template<class TTAsksList>
inline void OrderBook::setAsks(TTAsksList &&asks)
{
    _asks = std::forward<TAsksList>(asks);
}

using POrderBook = std::shared_ptr<OrderBook>;

}

Q_DECLARE_METATYPE(TradingCatCommon::POrderBook);
Q_DECLARE_METATYPE(TradingCatCommon::POrder);

