//My
#include <Common/parser.h>

#include "TradingCatCommon/userconfig.h"

using namespace TradingCatCommon;
using namespace Common;

UserConfig::UserConfig(const QString &json)
{
    if (json.isEmpty())
    {
        _filter = Filter();

        return;
    }

    try
    {
        const auto jsonDoc = Common::JSONParseToDocument(json.toUtf8());
        const auto configMap = JSONReadDocumentToMap(jsonDoc);
        const auto filterJson = JSONReadMapToMap(configMap, "Filter", "Filter");

        Filter filter(filterJson);
        if (filter.isError())
        {
            throw ParseException(QString("Error parsing filter configuration: %1").arg(filter.errorString()));
        }

        _filter = std::move(filter);
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
    }
}

QString UserConfig::toJson() const
{
    QJsonObject configMap;

    configMap.insert("Filter", _filter.toJson());

    return QJsonDocument(configMap).toJson(QJsonDocument::Compact);
}

void UserConfig::clearFilter() noexcept
{
    _filter.clear();
}

const Filter &UserConfig::filter() const noexcept
{
    return _filter;
}

bool TradingCatCommon::UserConfig::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &UserConfig::errorString() const noexcept
{
    return _errorString;
}
