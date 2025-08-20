#pragma once

//STL
#include <memory>
#include <list>

//Qt
#include <QtGlobal>

//My
#include <TradingCatCommon/symbol.h>

namespace TradingCatCommon
{

struct TradeStream
{
    enum class ETradeStreamType: quint8
    {
        UNDEFINED = 0,
        BUY = 1,
        SELL = 2
    };

    float price = 0.0f;
    float quantity = 0.0f;
    ETradeStreamType tradeType = ETradeStreamType::UNDEFINED;
    qint64 time = 0; // Epoch ms
};

using PTradeStream = std::shared_ptr<TradeStream>;

class TradeStreamData final
{
public:
    using TradeStreamList = std::list<PTradeStream>;
public:
    TradeStreamData(const TradingCatCommon::Symbol& symbol, qint64 time);
    ~TradeStreamData() = default;

    const TradeStreamList& tradeStreamList() const noexcept;

    const TradingCatCommon::Symbol& symbol() const noexcept;
    qint64 time() const noexcept;

    template <class TPTradeStream>
    void addTradeStream(TPTradeStream&& tradeStream);

private:
    TradeStreamData() = delete;
    Q_DISABLE_COPY_MOVE(TradeStreamData);

private:
    const TradingCatCommon::Symbol _symbol;
    const qint64 _time;

    TradeStreamList _tradeStreamList;

}; // class TradeStream

using PTradeStreamData = std::shared_ptr<TradeStreamData>;

template<class TPTradeStream>
inline void TradeStreamData::addTradeStream(TPTradeStream &&tradeStream)
{
    _tradeStreamList.push_back(std::forward<PTradeStream>(tradeStream));
}

} // namespace StockExchange

Q_DECLARE_METATYPE(TradingCatCommon::PTradeStreamData);
