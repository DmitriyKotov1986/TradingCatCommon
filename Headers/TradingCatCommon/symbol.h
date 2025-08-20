#pragma once

//STL
#include <memory>
#include <unordered_set>

//Qt
#include <QString>
#include <QMetaType>

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     The Symbol class - название монеты
///
struct Symbol
{
    QString name; ///< Полное название монеты полученно еот биржи

    /*!
        Конструктор по умолчанию. Сознает пустое название монеты
    */
    Symbol() = default;

    /*!
        Основной конструктор
        @param aname - полное название монеты
    */
    Symbol(const QString& aname);

    /*!
        Конструкторы копирования/перемещения
        @param id - исходный Символ
    */
    Symbol(const TradingCatCommon::Symbol& symbol);
    Symbol& operator=(const TradingCatCommon::Symbol& symbol);
    Symbol(TradingCatCommon::Symbol&& symbol);
    Symbol& operator=(TradingCatCommon::Symbol&& symbol);

    /*!
        Возвращает true если названеи монеты пустое
        @return true если данный иднтификатор пустой, false - иначе
    */
    bool isEmpty() const noexcept;

    /*!
        преобразует ИД свечи в строку
        @return ИД свечи в строковом представлении
    */
    QString toString() const;

    /*!
        Возвращает название базового инструмента (BTC_USDT вернет BTC)
        @return название базового инструмента
     */
    const QString& baseName() const;

private:
    mutable std::optional<QString> _baseName;  ///< Название базовой монеты

};

using SymbolsList = std::unordered_set<Symbol>;
using PSymbolsList = std::shared_ptr<SymbolsList>;

/*!
    Расчитывет назчания свечей для использования в контейнерах Qt
*/
size_t qHash(const TradingCatCommon::Symbol& key, size_t seed);

/*!
    Оператор сравнения двух названий монет
    @param key1
    @param key2
    @return true - если названия и типы свечей совпадают
 */
bool operator==(const TradingCatCommon::Symbol& key1, const TradingCatCommon::Symbol& key2);

/*!
    Оператор сравнения двух ИД свечей
    @param key1
    @param key2
    @return true - если названия и типы свечей не совпадают
 */
bool operator!=(const TradingCatCommon::Symbol& key1, const TradingCatCommon::Symbol& key2);

}  // namespace TradingCatCommon

//HASH function for std container
namespace std
{

template<>
struct hash<TradingCatCommon::Symbol>
{
    size_t operator()(const TradingCatCommon::Symbol& key) const noexcept;
};

} //namespace std

Q_DECLARE_METATYPE(TradingCatCommon::Symbol);
