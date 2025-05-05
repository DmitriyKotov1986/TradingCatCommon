#include "TradingCatCommon/istockexchange.h"

#ifndef QT_NO_SSL

using namespace TradingCatCommon;

IStockExchange::IStockExchange(const TradingCatCommon::StockExchangeID &id, QObject* parent /* = nullptr */)
    : QObject{parent}
    , _id(id)
{
    Q_ASSERT(!id.isEmpty());

    qRegisterMetaType<TradingCatCommon::StockExchangeID>("TradingCatCommon::StockExchangeID");
}

const TradingCatCommon::StockExchangeID &IStockExchange::id() const
{
    return _id;
}

void IStockExchange::start()
{
}

void IStockExchange::stop()
{
    emit finished();
}

#endif
