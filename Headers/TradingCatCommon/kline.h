#pragma once

//STL
#include <list>
#include <memory>
#include <unordered_set>

//QT
#include <QDateTime>
#include <QVector>
#include <QString>
#include <QSet>
#include <QHostAddress>
#include <QJsonObject>

namespace TradingCatCommon
{

/*!
    Тип свечей (интервалы). Значение соотвестввует интервалу свечи в мсек
 */
enum class KLineType: qint64
{
    MIN1   = 1 * 60 * 1000,     ///< 1мин свеча
    MIN5   = 5 * 60 * 1000,     ///< 5мин свеча
    MIN10  = 10 * 60 * 1000,    ///< 10мин свеча
    MIN15  = 15 * 60 * 1000,    ///< 15мин свеча
    MIN30  = 30 * 60 * 1000,    ///< 30мин свеча
    MIN60  = 60 * 60  * 1000,   ///< 60мин свеча
    HOUR4  = 240 * 60 * 1000,   ///< 4 часа
    HOUR8  = 480 * 60 * 1000,   ///< 8 часов
    DAY1   = 1440 * 60 * 1000,  ///< 1 день
    WEEK1  = 10080 * 60 * 1000, ///< 1 неделя
    UNDEFINED = 0               ///< неизвестный тип свечи
};

/*!
    Множество типов свечей. Обычно используется для передачи данных о типах свечей поддерживаемых биржей
*/
using KLineTypes = std::unordered_set<KLineType>;

/*!
    Преобразует список типов свечей в строку
    @param types - список тиаов свечей
    @return - строка
*/
QString klineTypesToString(const KLineTypes& types);

/*!
    Преобразует строку в список типов свечей
    @param types - строка
    @return - список типов свечей
*/
KLineTypes stringToKLineTypes(const QString& types);

/*!
    Преобразвет тип свечи в строку. Функция выполняет обратное преобразование к stringToKLineType(...)
    @param type - тип свечи
    @return строковео представление типа свечи
*/
QString KLineTypeToString(KLineType type);

/*!
    Преобразует строковое представление типа свечи в KLineType. Функция выполняет обратное преобразование к KLineTypeToString(...)
    @param type -тип свечи
    @return KLineType
*/
KLineType stringToKLineType(const QString& type) noexcept;

/*!
    Преобразвет число в тип свечи. Обратно преобразование можно выполнить с помощью static_cast
    @param type - числовое представление типа свечи
    @return  KLineType
*/
KLineType intToKLineType(qint64 type) noexcept;

///////////////////////////////////////////////////////////////////////////////
///     The KLineID class - идентификатор счечи
///
struct KLineID
{
    QString symbol;                         ///< название монеты
    KLineType type = KLineType::UNDEFINED;  ///< интервал свечи

    /*!
        Конструктор. Создает пустой идентификатор.
     */
    KLineID() = default;

    /*!
        Основной конструктор
        @param asymbol - название монеты. Не должно быть пустым
        @param atype - тим монеты. Не дожен быть равен KLineType::UNDEFINED
    */
    explicit KLineID(const QString& asymbol, KLineType atype);

    /*!
        Возвращает true если данный иднтификатор пустой (symbol.isEmpry() или type == KLineType::UNDEFINED
        @return true если данный иднтификатор пустой, false - иначе
    */
    bool isEmpty() const noexcept;

    /*!
        преобразует ИД свечи в строку
        @return ИД свечи в строковом представлении
    */
    QString toString() const;
};

} //namespace TradingCatCommon

//HASH function for std container
namespace std
{

template<>
struct hash<TradingCatCommon::KLineID>
{
    size_t operator()(const TradingCatCommon::KLineID& key) const noexcept;
};

} //namespace std

namespace TradingCatCommon
{

/*!
    Список идентификаторов свечей
*/
using KLinesIDList = std::unordered_set<KLineID>;

/*!
    Расчитывет хеш ИД свечей для использования в контейнерах Qt
*/
size_t qHash(const TradingCatCommon::KLineID& key, size_t seed);

/*!
    Оператор сравнения двух ИД свечей
    @param key1
    @param key2
    @return true - если названия и типы свечей совпадают
 */
bool operator==(const TradingCatCommon::KLineID& key1, const TradingCatCommon::KLineID& key2);

/*!
    Данные свечи
 */
struct KLine
{
    float open = 0.0f;     ///< цена в момент открытия
    float high = 0.0f;     ///< наибольшая свеча
    float low = 0.0f;      ///< наименьшая свеча
    float close = 0.0f;    ///< цена в момент закрытия
    float volume = 0.0f;   ///< объем
    float quoteAssetVolume = 0.0f;
    qint64 openTime;     ///< время открытия. мсек Epoch
    qint64 closeTime;    ///< время закрытия. мсек Epoch

    KLineID id;             ///< Идентификатор свечи

    /*!
        Возвращает true если данные свечи корректны
        @return  true если данные свечи корректны
    */
    bool check() const noexcept;

    /*!
        Возвращает приведенное изменение цены. Если low=0, то возвраает 0.0
        @return приведенное изменение цены
    */
    double deltaKLine() const noexcept;

    /*!
        Возвращает приведенное изменение объема
        @return приведенное изменение объема
    */
    double volumeKLine() const noexcept;

private:
    mutable std::optional<double> _delta;
    mutable std::optional<double> _volume;
};

/*!
    Указатель на свечу
*/
using PKLine = std::shared_ptr<KLine>;

/*!
    Список свечей
*/
using KLinesList = std::list<PKLine>;

/*!
    Указатель на список свечей
*/
using PKLinesList = std::shared_ptr<KLinesList>;

/*!
    Возвращает имя таблицы для хранения данных свечей
    @param stockExcangeName - название биржи
    @param moneyName - название монеты
    @param typeName - название интервала монеты
    @return Название таблицы
*/
QString getKLineTableName(const QString &stockExcangeName, const QString &moneyName, const QString &typeName);

} //namespace TraidingCatBot

Q_DECLARE_METATYPE(TradingCatCommon::KLine);
Q_DECLARE_METATYPE(TradingCatCommon::KLineID);
Q_DECLARE_METATYPE(TradingCatCommon::KLinesIDList);
Q_DECLARE_METATYPE(TradingCatCommon::KLinesList);
Q_DECLARE_METATYPE(TradingCatCommon::PKLinesList);




