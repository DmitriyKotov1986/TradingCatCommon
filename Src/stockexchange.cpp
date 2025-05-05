//Qt
#include <QHash>

#include "TradingCatCommon/stockexchange.h"

using namespace TradingCatCommon;

size_t TradingCatCommon::qHash(const TradingCatCommon::StockExchangeID &key, size_t seed)
{
    Q_UNUSED(seed);

    return qHash(key.name);
}

bool TradingCatCommon::operator==(const TradingCatCommon::StockExchangeID& key1, const TradingCatCommon::StockExchangeID& key2)
{
    return key1.name == key2.name;
}

std::size_t std::hash<TradingCatCommon::StockExchangeID>::operator()(const TradingCatCommon::StockExchangeID &key) const noexcept
{
    return static_cast<size_t>(TradingCatCommon::qHash(key, 0));
}

StockExchangeID::StockExchangeID(const QString &aname)
    : name(aname)
{
}

bool StockExchangeID::isEmpty() const noexcept
{
    return name.isEmpty();
}

QString StockExchangeID::toString() const
{
    return QString("%1").arg(name);
}
