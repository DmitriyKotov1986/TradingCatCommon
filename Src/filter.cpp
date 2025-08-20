//My
#include <Common/parser.h>

#include "TradingCatCommon/filter.h"

using namespace TradingCatCommon;
using namespace Common;

QString Filter::filterTypeToString(FilterType filterType)
{
    switch (filterType)
    {
    case Filter::FilterType::UNDETECT: return "UNDEFINED";
    case Filter::FilterType::DELTA: return "DELTA";
    case Filter::FilterType::VOLUME: return "VOLUME";
    default:
        Q_ASSERT(false);
    }

    return "UNDEFINED";
}

QString Filter::filterTypesToString(FilterTypes filterTypes)
{
    QStringList result;

    if (filterTypes.testFlag(Filter::FilterType::UNDETECT))
    {
        result.push_back(filterTypeToString(Filter::FilterType::UNDETECT));
    }
    if (filterTypes.testFlag(Filter::FilterType::DELTA))
    {
        result.push_back(filterTypeToString(Filter::FilterType::DELTA));
    }
    if (filterTypes.testFlag(Filter::FilterType::VOLUME))
    {
        result.push_back(filterTypeToString(Filter::FilterType::VOLUME));
    }

    return result.join(',');
}

Filter::Filter()
{
    KLineFilterData klineFilterData;
    klineFilterData.setDelta(KLineFilterData::DefaultDelta);
    klineFilterData.setVolume(KLineFilterData::DefaultVolume);

    _klineFilterData.emplace_back(std::move(klineFilterData));
}

Filter::Filter(const QJsonObject &JSONFilter)
{
    try
    {
        if (JSONFilter.contains("Filters"))
        {

            const auto klinesFilterJson = JSONReadMapToArray(JSONFilter, "Filters", "Filters");
            for (const auto& klineFilter: klinesFilterJson)
            {
                const auto filterDataJson = JSONReadMap(klineFilter, "Filters/[]");
                KLineFilterData klineFilterData(filterDataJson);

                if (klineFilterData.isError())
                {
                    throw ParseException(QString("Error parse filter: %1").arg(klineFilterData.errorString()));
                }

                _klineFilterData.emplace_back(std::move(klineFilterData));
            }
        }
        if (JSONFilter.contains("BlackList"))
        {
            const auto blackListFilterJson = JSONReadMapToArray(JSONFilter, "BlackList", "BlackList");
            for (const auto& blackListFilter: blackListFilterJson)
            {
                const auto blackListFilterDataJson = JSONReadMap(blackListFilter, "BlackList/[]");
                BlackListFilterData blackListData(blackListFilterDataJson);

                if (blackListData.isError())
                {
                    throw ParseException(QString("Error parse black list: %1").arg(blackListData.errorString()));
                }

                _blackListFilterDataList.emplace_back(std::move(blackListData));
            }
        }
    }
    catch (const ParseException& err)
    {
        _klineFilterData.clear();
        _blackListFilterDataList.clear();

        _errorString = err.what();

        return;
    }
}

bool Filter::isError() const noexcept
{
    return !_errorString.isEmpty();
}

QString Filter::errorString()
{
    const auto err = _errorString;
    _errorString.clear();

    return err;
}

const Filter::KLineFilterDataList& Filter::klineFilter() const noexcept
{
    return _klineFilterData;
}

qsizetype Filter::klineFilterCount() const noexcept
{
    return _klineFilterData.size();
}

QJsonObject Filter::toJson() const
{
    QJsonObject json;

    QJsonArray klineJson;
    for (const auto& klineFilterData: _klineFilterData)
    {
        klineJson.push_back(klineFilterData.toJson());
    }
    json.insert("Filters", klineJson);

    QJsonArray blackListJson;
    for (const auto& blackListData: _blackListFilterDataList)
    {
        blackListJson.push_back(blackListData.toJson());
    }
    json.insert("BlackList", blackListJson);

    return json;
}

void Filter::clear() noexcept
{
    _klineFilterData.clear();
    _blackListFilterDataList.clear();
}

const Filter::BlackListFilterDataList &Filter::blackList() const noexcept
{
    return _blackListFilterDataList;
}

