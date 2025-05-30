#pragma once

//STL
#include <list>
#include <utility>

//Qt
#include <QFlags>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>

//My
#include "TradingCatCommon/klinefilterdata.h"
#include "TradingCatCommon/blacklistfilterdata.h"

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///  Filter класс-контейнер хранит параметры фильтра свечей
///
class Filter final
{
public:
    /*!
        Тип стработавшего фильра.
     */
    enum FilterType
    {
        UNDETECT =    0x1, ///< Фильтр не сработал
        DELTA =       0x2, ///< Сработал фильтр на Дельту цены
        VOLUME =      0x4  ///< Сработал фильтр на Дельту объема
    };

    /*!
        Т.к. возможно срабатывание по нескольким критериям, объявлем Q_FLAGS
    */
    Q_DECLARE_FLAGS(FilterTypes, Filter::FilterType);

    static QString filterTypeToString(FilterType filterType);
    static QString filterTypesToString(FilterTypes filterTypes);

    /*!
        Список правил фильтрации
    */
    using KLineFilterDataList = std::list<KLineFilterData>;

    /*!
        Блеклист
    */
    using BlackListFilterDataList = std::list<BlackListFilterData>;

public:
    Filter();
    Filter(const Filter& filter) = default;
    Filter& operator=(const Filter& filter) = default;
    Filter(Filter&& filter) = default;
    Filter& operator=(Filter&& filter) = default;

    /*!
        Создает фильр из ранее созданного JSON.
            Если при парсинге json произошла ошибка - isError() - вернет true. Текст ошибки можно вернуть с помощью errorString()
        @param json - json представление фильтра
    */
    explicit Filter(const QJsonObject& json);

    /*!
        Сохраняет текущие правила фильрации в json
    */
    QJsonObject toJson() const;

    /*!
        Очищает список правил фильтра
    */
    void clear() noexcept;

    /*!
        Возвращает текущие правил фильтрации
        @return - текущие правил фильтрации
     */
    const KLineFilterDataList& klineFilter() const noexcept;

    /*!
        Добавляет правило фильтрации
        @param filterData - правило фильтрации
    */
    template <class TKLineFilterData>
    void addFilterData(TKLineFilterData&& filterData);

    /*!
        Возвращает текущиq блек-лист
        @return - текущие правил фильтрации
     */
    const BlackListFilterDataList& blackList() const noexcept;

    /*!
        Добавляет правило в блек-лист
        @param filterData - правило фильтрации
    */
    template <class TBlackListFilterDataList>
    void addBlackListData(TBlackListFilterDataList&& blackList);

    /*!
        Возвращает true - если последний парсинг json закончился ошибкой
        @return true - была ошибка парсинга
    */
    bool isError() const noexcept;

    /*!
        Текстовое описание последней ошибки парсинга json. При вызове этого метода происходит сброс ошибки
        @return текст ошибки
     */
    [[nodiscard]] QString errorString();

    /*!
        Возвращает количество правил в фильтре
        @return количество правил
    */
    qsizetype klineFilterCount()  const noexcept;

private:
    KLineFilterDataList _klineFilterData; ///< Список правил фильрации

    BlackListFilterDataList _blackListFilterDataList; ///< Блек-лист

    QString _errorString;       ///< Текст текущей ошибки парсинга

};

template <class TKLineFilterData>
void Filter::addFilterData(TKLineFilterData&& filterData)
{
    _klineFilterData.push_back(std::forward<KLineFilterData>(filterData));
}

template <class TBlackListFilterData>
void Filter::addBlackListData(TBlackListFilterData&& blackList)
{
    _blackListFilterDataList.push_back(std::forward<BlackListFilterData>(blackList));
}

} //namespace TradingCatCommon

Q_DECLARE_OPERATORS_FOR_FLAGS(TradingCatCommon::Filter::FilterTypes);

Q_DECLARE_METATYPE(TradingCatCommon::Filter::FilterTypes);
Q_DECLARE_METATYPE(TradingCatCommon::Filter);
