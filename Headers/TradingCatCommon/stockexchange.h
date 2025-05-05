#pragma once

//STL
#include <unordered_set>

//Qt
#include <QString>
#include <QJsonObject>

namespace TradingCatCommon
{

struct StockExchangeID
{
    QString name;

    StockExchangeID() = default;
    StockExchangeID(const StockExchangeID& stockExchangeID) = default;
    StockExchangeID& operator=(const StockExchangeID& stockExchangeID) = default;
    StockExchangeID(StockExchangeID&& stockExchangeID) = default;
    StockExchangeID& operator=(StockExchangeID&& stockExchangeID) = default;

    StockExchangeID(const QString& aname);

    bool isEmpty() const noexcept;

    QString toString() const;
};

size_t qHash(const TradingCatCommon::StockExchangeID& key, size_t seed);
bool operator==(const TradingCatCommon::StockExchangeID& key1, const TradingCatCommon::StockExchangeID& key2);

} // namespace TradingCatCommon

//HASH function for std container
namespace std
{

template<>
struct hash<TradingCatCommon::StockExchangeID>
{
    size_t operator()(const TradingCatCommon::StockExchangeID& key) const noexcept;
};

} //namespace std

namespace TradingCatCommon
{

using StockExchangesIDList = std::unordered_set<StockExchangeID>;

} // namespace TradingCatCommon

Q_DECLARE_METATYPE(TradingCatCommon::StockExchangeID);
Q_DECLARE_METATYPE(TradingCatCommon::StockExchangesIDList);


