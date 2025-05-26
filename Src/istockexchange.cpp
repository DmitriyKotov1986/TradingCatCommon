#include "TradingCatCommon/istockexchange.h"

#ifndef QT_NO_SSL

using namespace TradingCatCommon;

IStockExchange::IStockExchange(const TradingCatCommon::StockExchangeID &stockExchangeId, QObject* parent /* = nullptr */)
    : QObject{parent}
    , _stockExchangeId(stockExchangeId)
{
    Q_ASSERT(!stockExchangeId.isEmpty());

    qRegisterMetaType<TradingCatCommon::StockExchangeID>("TradingCatCommon::StockExchangeID");
}

const TradingCatCommon::StockExchangeID &IStockExchange::id() const
{
    return _stockExchangeId;
}

void IStockExchange::start()
{
}

void IStockExchange::stop()
{
    emit finished();
}

#endif
