#pragma once

//Qt
#include <QString>

//My
#include "TradingCatCommon/filter.h"

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     The UserConfig class - класс конфигурации пользователя настраиваемой
///         через приложение
///
struct UserConfig final
{
public:
    UserConfig() = default;
    UserConfig(const UserConfig& userConfig) = default;
    UserConfig& operator=(const UserConfig& userConfig) = default;
    UserConfig(UserConfig&& userConfig) = default;
    UserConfig& operator=(UserConfig&& userConfig) = default;

    explicit UserConfig(const QString& json);

    QString toJson() const;

    /*!
        Возвращет фильтр
        @return фильтр
    */
    const TradingCatCommon::Filter& filter() const noexcept;

    /*!
        Очищает список правила фильтра
    */
    void clearFilter() noexcept;

    /*!
        Добавляет правило фильтрации
        @param filterData - правило фильтрации
    */
    template <class TKLineFilterData>
    void addFilterData(TKLineFilterData&& filterData);

    /*!
        Добавляет правило фильтрации
        @param filterData - правило фильтрации
    */
    void addFilterData(KLineFilterData&& filterData);

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    TradingCatCommon::Filter _filter;

}; // struct UserConfig

template <class TKLineFilterData>
void UserConfig::addFilterData(TKLineFilterData&& filterData)
{
    _filter.addFilterData(std::forward<KLineFilterData>(filterData));
}

using PUserConfig = std::shared_ptr<TradingCatCommon::UserConfig>;

} // namespace TradingCatCommon

Q_DECLARE_METATYPE(TradingCatCommon::UserConfig)
Q_DECLARE_METATYPE(TradingCatCommon::PUserConfig);
