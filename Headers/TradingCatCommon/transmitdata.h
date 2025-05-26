#pragma once

//Qt
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>

//My
#include <Common/parser.h>

#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"

namespace TradingCatCommon
{

/*!
    Типы пакетов в протоколе
*/
enum class PackageType: quint8
{
    UNDEFINED = 0,              ///< Неопределен
    NULL_DATA = 1,              ///< Нулевой пакет без данных
    SERVER_STATUS = 2,          ///< Пакет статуса сервера
    LOGIN = 3,                  ///< Логин
    LOGOUT = 4,                 ///< Логаут
    CONFIG = 5,                 ///< Сохранение конфигурации пользователя
    STOCKEXCHANGES = 6,         ///< Список поддерживаемых бирж
    KLINESIDLIST = 7,           ///< Список доступных на бирже свечей
    DETECT = 8                  ///< Список свечей прошедших фильтр
};

Q_GLOBAL_STATIC_WITH_ARGS(const QString, OK_ANSWER_TEXT, ("OK")); ///< Сообщение об успешной обработке данных

///////////////////////////////////////////////////////////////////////////////
/// Запросы на сервер
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///     The Query class базовй класс запроса
///
class Query
{
public:
    /*!
        Конструктор. Предполагается использовать только этот конструктор
        @param type - тип запроса
        @param path - пцть запроса на сервере. Например /path/json. Обязательно должен начинаться с / и заканчиваться символом
    */
    Query(PackageType type, const QString& path);

    /*!
        Деструктор
    */
    virtual ~Query();

    /*!
        Возвращает тип запроса
        @return тип завпроса
    */
    PackageType type() const noexcept;

    /*!
        Путь запроса на сервере
        @return - путь запроса на сервере
    */
    const QString& path() const noexcept;

    /*!
        Метод должен возвращаеть адрес запроса на сервер с учетом дополнительных параметров
        @return
    */
    virtual QUrlQuery query() const;

    /*!
        Возвращает идентификатор запроса. Если ИД не установлен вручную setID() то он автоматически увеличивается на 1
        @return Уникальный идентификатор запроса
    */
    quint64 id() const noexcept;

    /*!
        Устанавливает идентификатор запроса. Идентификатор должен быть установлен только оди раз при первой
            отправке запроса.
        @param id - идентификатор запроса. Не должет быть равен 0
     */
    void setID(quint64 id) noexcept;

    /*!
        Возвращает true если идетификатор запроса еще не был установлен
        @return  true если идетификатор запроса еще не был установлен
    */
    bool isFirstUse() const noexcept;

private:
    // Удаляем неиспользуемые конструторы
    Query() = delete;
    Q_DISABLE_COPY_MOVE(Query);

private:
    const PackageType _type = PackageType::UNDEFINED; ///< Тип пакета
    const QString _path;                              ///< Путь запроса на сервере

    quint64 _id = 0;                                  ///< ИД запроса
};

///////////////////////////////////////////////////////////////////////////////
///  The StatusJson class - статус обрабоки запроса сервером
///
class StatusAnswer
{
public:
    /*!
        Коды ошибок обработки запросов
     */
    enum class ErrorCode: quint16
    {
        UNDEFINED = 0,          ///< Неопределенное состояние
        OK = 200,               ///< Запрос выполнен успешно
        BAD_REQUEST = 400,      ///< Неверный запрос
        UNAUTHORIZED = 401,     ///< Пользователь на авторизован
        NOT_FOUND = 404,        ///< Данные не найдены
        REQUEST_TIMEOUT = 408,  ///< Таймаут обработки запроса
        INTERNAL_ERROR = 500    ///< Внутренняя ошибка сервера
    };

    /*!
        Преобразует код обработки запроса в текстовое описание
        @param code -  Коды ошибок обработки запросов
        @return Строка с описанием ошибки
    */
    static QString errorCodeToStr(ErrorCode code);

    /*!
        Преобразует число в код обработки запроса
        @param code -  код ошибки в виде числа. Если Число не соотвествует ни одному из ErrorCode, то метод вернет ErrorCode::UNDEFINED
        @return Код ошибоки обработки запросов
    */
    static ErrorCode intToErrorCode(quint16 code);

public:
    StatusAnswer() = default;
    StatusAnswer(const QJsonObject& json);
    StatusAnswer(ErrorCode code, const QString& msg = QString());

    /*!
        Сериализует статус в Json Object
        @return
     */
    QJsonObject toJson() const;

    /*!
        Возвращает true если код статуса равен ErrorCode::OK
     */
    operator bool() const;

    const QString& message() const noexcept;
    ErrorCode code() const noexcept;
    QString status() const;

private:
    ErrorCode _code = ErrorCode::OK;  ///< Код статуса
    QString _msg;                     ///< Пояснительное сообщение к коду

};

///////////////////////////////////////////////////////////////////////////////
/// The Package class - нулевой набор данных
///

class NullDataPackage
{
public:
    NullDataPackage() = default;
    NullDataPackage(const QJsonObject& json) noexcept
    {
        Q_UNUSED(json)
    };

    QJsonObject toJson() const noexcept
    {
        return {};
    };
};


///////////////////////////////////////////////////////////////////////////////
///     The Package class - шаблонный класс пакета данных в ответе сервера
///
template<class TPackageDataJson = NullDataPackage>
class Package
{
public:
    Package(StatusAnswer::ErrorCode code, const QString& msg = QString()) //в случае ошибки
        : _status(StatusAnswer(code, msg))
        , _data()
    {
    }

    Package(const TPackageDataJson& data) //В случае успеха)
        : _status(StatusAnswer::ErrorCode::OK)
        , _data(data)
    {
    }

    Package(const QByteArray& data)
    {
        using namespace Common;

        try
        {
            const auto jsonDoc = JSONParseToDocument(data);

            const auto json = JSONReadDocumentToMap(jsonDoc);

            const auto jsonStatus = JSONReadMapToMap(json, "Status", "Root/Status");
            _status = StatusAnswer(jsonStatus);

            if (_status)
            {
                const auto jsonData = json["Data"];
                if (jsonData.isNull() || jsonData.isUndefined())
                {
                    _errorString = QString("Root/Data cannot be empty");
                }

                _data = TPackageDataJson(jsonData);

            }
        }
        catch (const ParseException& err)
        {
            _errorString = err.what();
        }
    }

    QByteArray toJson() const
    {
        QJsonObject result;

        result.insert("Status", _status.toJson());

        if (_status)
        {
            result.insert("Data", _data.toJson());
        }

        return QJsonDocument(result).toJson(QJsonDocument::Compact) + "\n\r";
    }

    const TPackageDataJson& data() const noexcept
    {
        return _data;
    }

    const StatusAnswer& status() const noexcept
    {
        return _status;
    }

    bool isError() const noexcept
    {
        return !_errorString.isEmpty();
    }

    const QString& errorString() const noexcept
    {
        return _errorString;
    }

private:
    QString _errorString;

    StatusAnswer _status;
    TPackageDataJson _data;

};

///////////////////////////////////////////////////////////////////////////////
///     The KLineJson class
///
class KLineJson
{
public:
    explicit KLineJson(const TradingCatCommon::PKLine& kline);
    explicit KLineJson(const QJsonValue& json);

    QJsonObject toJson() const;

    const TradingCatCommon::PKLine& kline() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    KLineJson() = delete;

private:
    QString _errorString;

    TradingCatCommon::PKLine _kline;

};

///////////////////////////////////////////////////////////////////////////////
///     The KLinesArrayJson class - список свечей
///
class KLinesArrayJson
{
public:
    KLinesArrayJson(const PKLinesList& klinesList);
    KLinesArrayJson(const QJsonValue& json);

    QJsonArray toJson() const;

    const PKLinesList& klinesList() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    KLinesArrayJson() = delete;

private:
    QString _errorString;

    PKLinesList _klinesList;

};

///////////////////////////////////////////////////////////////////////////////
///     The class KLineIDJson
///
class KLineIDJson final
{
public:
    KLineIDJson(const TradingCatCommon::KLineID& klineId);
    KLineIDJson(const QJsonValue& json);

    QJsonObject toJson() const;

    const TradingCatCommon::KLineID& klineId() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    KLineIDJson() = delete;

private:
    QString _errorString;

    TradingCatCommon::KLineID _klineId;

};

///////////////////////////////////////////////////////////////////////////////
///     The class StockExchangeIDJson
///
class StockExchangeIDJson final
{
public:
    StockExchangeIDJson(const TradingCatCommon::StockExchangeID& stockExchangeId);
    StockExchangeIDJson(const QJsonValue& json);

    QJsonObject toJson() const;

    const TradingCatCommon::StockExchangeID& stockExchangeId() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    StockExchangeIDJson() = delete;

private:
    QString _errorString;

    TradingCatCommon::StockExchangeID _stockExchangeId;

};

///////////////////////////////////////////////////////////////////////////////
///     The StockExchangesIDArrayJson class
///
class StockExchangesIDArrayJson final
{
public:
    explicit StockExchangesIDArrayJson(const QJsonValue& json);
    explicit StockExchangesIDArrayJson(const TradingCatCommon::StockExchangesIDList& stockExchangesIDList);

    QJsonArray toJson() const;

    TradingCatCommon::StockExchangesIDList stockExchagesIDList() const;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    StockExchangesIDList _stockExchangesIDList;

};

} // namespace TradingCatCommon
