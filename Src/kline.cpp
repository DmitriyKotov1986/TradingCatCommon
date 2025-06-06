//STL
#include <limits>
#include <cmath>

//Qt
#include <QRegularExpression>

//My
#include <Common/common.h>

#include "TradingCatCommon/kline.h"

using namespace TradingCatCommon;
using namespace Common;

Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_UNDEFINED, ("UNDEFINED"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_MIN1, ("1m"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_MIN5, ("5m"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_MIN10, ("10m"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_MIN15, ("15m"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_MIN30, ("30m"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_MIN60, ("60m"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_HOUR4, ("4h"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_HOUR8, ("8h"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_DAY1, ("1d"));
Q_GLOBAL_STATIC_WITH_ARGS(const QString, KLINETYPE_WEEK1, ("1w"));

static const double POSITIV_EPSILON = -std::numeric_limits<double>::epsilon();

QString TradingCatCommon::KLineTypeToString(KLineType type)
{
    switch (type)
    {
    case KLineType::UNDEFINED: return *KLINETYPE_UNDEFINED;
    case KLineType::MIN1:  return *KLINETYPE_MIN1;
    case KLineType::MIN5:  return *KLINETYPE_MIN5;
    case KLineType::MIN10: return *KLINETYPE_MIN10;
    case KLineType::MIN15: return *KLINETYPE_MIN15;
    case KLineType::MIN30: return *KLINETYPE_MIN30;
    case KLineType::MIN60: return *KLINETYPE_MIN60;
    case KLineType::HOUR4: return *KLINETYPE_HOUR4;
    case KLineType::HOUR8: return *KLINETYPE_HOUR8;
    case KLineType::DAY1:  return *KLINETYPE_DAY1;
    case KLineType::WEEK1: return *KLINETYPE_WEEK1;
    default:
        Q_ASSERT(false);
    }

    return *KLINETYPE_UNDEFINED;
}

KLineType TradingCatCommon::stringToKLineType(const QString& type) noexcept
{
    if (type == *KLINETYPE_MIN1)
    {
        return KLineType::MIN1;
    }
    else if (type == *KLINETYPE_MIN5)
    {
        return KLineType::MIN5;
    }
    else if (type == *KLINETYPE_MIN10)
    {
        return KLineType::MIN10;
    }
    else if (type == *KLINETYPE_MIN15)
    {
        return KLineType::MIN15;
    }
    else if (type == *KLINETYPE_MIN30)
    {
        return KLineType::MIN30;
    }
    else if (type == *KLINETYPE_MIN60)
    {
        return KLineType::MIN60;
    }
    else if (type == *KLINETYPE_HOUR4)
    {
        return KLineType::HOUR4;
    }
    else if (type == *KLINETYPE_HOUR8)
    {
        return KLineType::HOUR8;
    }
    else if (type == *KLINETYPE_DAY1)
    {
        return KLineType::DAY1;
    }
    else if (type == *KLINETYPE_WEEK1)
    {
        return KLineType::WEEK1;
    }

    return KLineType::UNDEFINED;
}

KLineType intToKLineType(qint64 type) noexcept
{
    if (type == static_cast<qint64>(KLineType::MIN1))
    {
        return KLineType::MIN1;
    }
    else if (type == static_cast<qint64>(KLineType::MIN5))
    {
        return KLineType::MIN5;
    }
    else if (type == static_cast<qint64>(KLineType::MIN10))
    {
        return KLineType::MIN10;
    }
    else if (type == static_cast<qint64>(KLineType::MIN15))
    {
        return KLineType::MIN15;
    }
    else if (type == static_cast<qint64>(KLineType::MIN30))
    {
        return KLineType::MIN30;
    }
    else if (type == static_cast<qint64>(KLineType::MIN60))
    {
        return KLineType::MIN60;
    }
    else if (type == static_cast<qint64>(KLineType::HOUR4))
    {
        return KLineType::HOUR4;
    }
    else if (type == static_cast<qint64>(KLineType::HOUR8))
    {
        return KLineType::HOUR8;
    }
    else if (type == static_cast<qint64>(KLineType::DAY1))
    {
        return KLineType::DAY1;
    }
    else if (type == static_cast<qint64>(KLineType::WEEK1))
    {
        return KLineType::WEEK1;
    }

    return KLineType::UNDEFINED;
}

size_t TradingCatCommon::qHash(const KLineID &key, size_t seed)
{
    Q_UNUSED(seed);

    return qHash(key.symbol, 0) + 47 * static_cast<quint64>(key.type);
}

bool TradingCatCommon::operator==(const KLineID &key1, const KLineID &key2)
{
    return (key1.symbol == key2.symbol) && (key1.type == key2.type);
}

bool TradingCatCommon::operator!=(const KLineID &key1, const KLineID &key2)
{
    return !(key1 == key2);
}

bool TradingCatCommon::operator==(const Symbol& key1, const Symbol& key2)
{
    return key1.name == key2.name;
}

bool TradingCatCommon::operator!=(const Symbol& key1, const Symbol& key2)
{
    return !(key1 == key2);
}

std::size_t std::hash<KLineID>::operator()(const KLineID &key) const noexcept
{
    return static_cast<size_t>(TradingCatCommon::qHash(key, 0));
}

bool KLineID::isEmpty() const noexcept
{
    return (symbol.isEmpty()) || (type == KLineType::UNDEFINED);
}

QString KLineID::toString() const
{
    return QString("%1:%2").arg(symbol.name).arg(KLineTypeToString(type));
}

const QString &KLineID::baseName() const
{
    return symbol.baseName();
}

KLineID::KLineID(const QString &asymbol, KLineType atype)
    : symbol(asymbol)
    , type(atype)
{
}

KLineID::KLineID(const TradingCatCommon::KLineID& id)
    : symbol(id.symbol)
    , type(id.type)
{
}

KLineID &KLineID::operator=(const KLineID& id)
{
    symbol = id.symbol;
    type = id.type;

    return *this;
}

KLineID::KLineID(KLineID&& id)
    : symbol(std::move(id.symbol))
    , type(std::move(id.type))
{
}

KLineID& KLineID::operator=(KLineID&& id)
{
    symbol = std::move(id.symbol);
    type = std::move(id.type);

    return *this;
}

QString TradingCatCommon::getKLineTableName(const QString &stockExcangeName, const QString &moneyName, const QString &typeName)
{
    Q_UNUSED(moneyName);

    return QString("KLines_%1_%2").arg(stockExcangeName).arg(typeName);
}

bool KLine::check() const noexcept
{
    const auto curDateTime = QDateTime::currentDateTime().addDays(1).toMSecsSinceEpoch();
    return !id.isEmpty() &&
           closeTime > openTime &&
           closeTime < curDateTime &&
           openTime < curDateTime &&
           open > POSITIV_EPSILON &&
           high > POSITIV_EPSILON &&
           low > POSITIV_EPSILON &&
           close > POSITIV_EPSILON &&
           volume > POSITIV_EPSILON &&
           quoteAssetVolume > POSITIV_EPSILON;
}

double TradingCatCommon::KLine::deltaKLine() const noexcept
{
    if (_delta.has_value())
    {
        return _delta.value();
    }

    if (std::fabs(low) < std::numeric_limits<double>::epsilon())
    {
        _delta = 0.0f;
    }
    else
    {
        _delta = ((high - low) / low) * 100.0f;
        Q_ASSERT(_delta > POSITIV_EPSILON);
    }

    return _delta.value();
}

double TradingCatCommon::KLine::volumeKLine() const noexcept
{
    if (!_volume.has_value())
    {
        _volume = ((open + close) / 2.0f) * volume;
    }

    return _volume.value();
}

const QString &KLine::toString() const
{
    if (!_data.has_value())
    {
        _data = QString("ID: %1. Open time: %2 Close time: %3 High: %4 Open: %5 Close: %6 Low: %7 Volume: %8 Quote asset volume: %9")
            .arg(id.toString())
            .arg(QDateTime::fromMSecsSinceEpoch(openTime).toString(DATETIME_FORMAT))
            .arg(QDateTime::fromMSecsSinceEpoch(closeTime).toString(DATETIME_FORMAT))
            .arg(high)
            .arg(open)
            .arg(close)
            .arg(low)
            .arg(volume)
            .arg(quoteAssetVolume);
    }

    return _data.value();
}


QString TradingCatCommon::klineTypesToString(const KLineTypes &types)
{
    QStringList result;

    for (const auto& type: types)
    {
        result.push_back(KLineTypeToString(type));
    }

    return result.join(u',');
}

KLineTypes TradingCatCommon::stringToKLineTypes(const QString &types)
{
    const auto stringList = types.split(u',');

    KLineTypes result;
    if (stringList.first().isEmpty())
    {
        return result;
    }

    for (const auto& typeStr: stringList)
    {
        result.emplace(stringToKLineType(typeStr));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
///     The Symbol class - название монеты
///
Symbol::Symbol(const QString &aname)
    : name(aname)
{
}

Symbol::Symbol(const Symbol &symbol)
    : name(symbol.name)
    , _baseName(symbol._baseName.has_value() ? std::make_optional(symbol._baseName.value()) : std::nullopt)
{
}

Symbol &Symbol::operator=(const Symbol &symbol)
{
    name = symbol.name;
    _baseName = symbol._baseName.has_value() ? std::make_optional(symbol._baseName.value()) : std::nullopt;

    return *this;
}

Symbol::Symbol(Symbol &&symbol)
    : name(std::move(symbol.name))
    , _baseName(symbol._baseName.has_value() ? std::make_optional(symbol._baseName.value()) : std::nullopt)
{
}

Symbol &Symbol::operator=(Symbol &&symbol)
{
    name = std::move(symbol.name);
    _baseName = symbol._baseName.has_value() ? std::make_optional(symbol._baseName.value()) : std::nullopt;

    return *this;
}

bool Symbol::isEmpty() const noexcept
{
    return name.isEmpty();
}

QString Symbol::toString() const
{
    return name;
}

const QString &Symbol::baseName() const
{
    if (_baseName.has_value())
    {
        return _baseName.value();
    }

    static const auto reg = QRegularExpression("[^A-Z0-9]");

    auto klineName = name;
    klineName = klineName.first(klineName.indexOf("USDT"));
    klineName = klineName.remove(reg);

    _baseName = klineName;

    return _baseName.value();
}

std::size_t std::hash<TradingCatCommon::Symbol>::operator()(const TradingCatCommon::Symbol &key) const noexcept
{
    return qHash(key, 0);
}

size_t TradingCatCommon::qHash(const Symbol &key, size_t seed)
{
    Q_UNUSED(seed);

    return qHash(key.name);
}
