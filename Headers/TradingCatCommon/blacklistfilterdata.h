#pragma once

//STL
#include <optional>

//Qt
#include <QString>
#include <QJsonObject>

//My
#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"

namespace TradingCatCommon
{

/*!
   Правило фильрации
*/
class BlackListFilterData final
{
public:
    BlackListFilterData() = default;
    BlackListFilterData(const BlackListFilterData& blackListFilterData);
    BlackListFilterData& operator=(const BlackListFilterData& blackListFilterData);
    BlackListFilterData(BlackListFilterData&& blackListFilterData);
    BlackListFilterData& operator=(BlackListFilterData&& blackListFilterData);

    explicit BlackListFilterData(const QJsonObject& json);

    const std::optional<TradingCatCommon::StockExchangeID>& stockExchangeID() const noexcept; ///< ИД Биржи
    const std::optional<TradingCatCommon::KLineID>& klineID() const noexcept;  ///< ИД Свечи или ALL_KLINESID, если фильтр относится ко всем свечам биржи

    void setStockExchangeID(const std::optional<TradingCatCommon::StockExchangeID>& stockExchangeId);
    void setKLineID(const std::optional<TradingCatCommon::KLineID>& klineId);

    /*!
        Очищает поля фильра
     */
    void clear();

    /*!
        Возвращает true, если текущие значения фильтра корректны
        @return true, если текущие значения фильтра корректны, false - некорректны
    */
    bool check() const noexcept;

    /*!
        Возвращает true если текущий фильр относится ко всей бирже stockExchangeID
        @return true - фильтр относится ко всей бирже, false  -  фильтр относится к
            определенной монете с klineID
    */
    bool isAllKLineID() const noexcept;

    /*!
        Возаращает true, если фильтр относится ко всем биржам
        @return  true - если фильтр относится ко всем биржам
    */
    bool isAllStockExchange() const noexcept;

    /*!
        Преобразует текущи фильр в json объект
        @return json представление текущего фильра
    */
    QJsonObject toJson() const;

    /*!
        Возвращает true - если последний парсинг json закончился ошибкой
        @return true - была ошибка парсинга
    */

    [[nodiscard]] bool isError() const noexcept;

    /*!
        Текстовое описание последней ошибки парсинга json. При вызове этого метода происходит сброс ошибки
        @return текст ошибки
    */
    [[nodiscard]] QString errorString();

private:
    //data
    std::optional<TradingCatCommon::StockExchangeID> _stockExchangeID = std::nullopt; ///< ИД Биржи
    std::optional<TradingCatCommon::KLineID> _klineID = std::nullopt;  ///< ИД Свечи или ALL_KLINESID, если фильтр относится ко всем свечам биржи

    //service
    QString _errorString;
};

} // namespace TradingCatCommon
