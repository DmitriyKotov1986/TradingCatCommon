#include "TradingCatCommon/iorderbook.h"

using namespace TradingCatCommon;

IOrderBook::IOrderBook(const TradingCatCommon::StockExchangeID& stockExchangeId, const QString& symbol, QObject *parent /* = nullptr */)
    : QObject{parent}
    , _stockExchangeId(stockExchangeId)
    , _symbol(symbol)
{
    Q_ASSERT(!_symbol.isEmpty());
}

const QString& IOrderBook::symbol() const
{
    return _symbol;
}

void IOrderBook::setHTTP(Common::HTTPSSLQuery* http)
{
    Q_CHECK_PTR(http);

    _http = http;
}

Common::HTTPSSLQuery *IOrderBook::getHTTP() const noexcept
{
    return _http;
}

void IOrderBook::setTradingData(TradingData *tradingData)
{
    Q_CHECK_PTR(tradingData);

    _tradingData = tradingData;
}

void TradingCatCommon::IOrderBook::start()
{
    Q_CHECK_PTR(_http);
}

void TradingCatCommon::IOrderBook::stop()
{
}

void IOrderBook::addOrderBook(const POrderBook &orderBook)
{
    Q_CHECK_PTR(_tradingData);

    if (!orderBook)
    {
        _tradingData->addOrderBook(_stockExchangeId, orderBook);
    }
}
