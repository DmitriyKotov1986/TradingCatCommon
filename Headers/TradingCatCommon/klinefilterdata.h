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
class KLineFilterData final
{
public:
    inline static const float MaxDelta = 1000000.0f;
    inline static const float DefaultDelta = 5.0f;
    inline static const float MinDelta = 1.0f;

    inline static const float MaxVolume = 1000000.0f;
    inline static const float DefaultVolume = 2000.0f;
    inline static const float MinVolume = 500.0f;

public:
    KLineFilterData() = default;
    KLineFilterData(const KLineFilterData& klineFilterData);
    KLineFilterData& operator=(const KLineFilterData& klineFilterData);
    KLineFilterData(KLineFilterData&& klineFilterData);
    KLineFilterData& operator=(KLineFilterData&& klineFilterData);

    explicit KLineFilterData(const QJsonObject& json);

    const std::optional<TradingCatCommon::StockExchangeID>& stockExchangeID() const noexcept; ///< ИД Биржи
    const std::optional<TradingCatCommon::KLineID>& klineID() const noexcept;  ///< ИД Свечи или ALL_KLINESID, если фильтр относится ко всем свечам биржи
    const std::optional<float>& delta() const noexcept;        ///< Пороговое значение дельты по цене
    const std::optional<float>& volume() const noexcept;       ///< Пороговое значение дельты по объему

    void setStockExchangeID(const std::optional<TradingCatCommon::StockExchangeID>& stockExchangeId);
    void setKLlineID( const std::optional<TradingCatCommon::KLineID>& klineId);
    void setDelta(const std::optional<float>& delta);
    void setVolume(const std::optional<float>& volume);

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
    std::optional<float> _delta = std::nullopt;        ///< Пороговое значение дельты по цене
    std::optional<float> _volume = std::nullopt;       ///< Пороговое значение дельты по объему

    //service
    QString _errorString;
};

} // namespace TradingCatCommon
