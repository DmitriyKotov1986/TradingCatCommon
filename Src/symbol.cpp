//Qt
#include <QRegularExpression>

#include <TradingCatCommon/symbol.h>

using namespace TradingCatCommon;

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

    auto klineName = name.toUpper();
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

bool TradingCatCommon::operator==(const Symbol& key1, const Symbol& key2)
{
    return key1.name == key2.name;
}

bool TradingCatCommon::operator!=(const Symbol& key1, const Symbol& key2)
{
    return !(key1 == key2);
}
