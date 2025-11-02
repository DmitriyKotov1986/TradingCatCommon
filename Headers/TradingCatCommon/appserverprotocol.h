#pragma once

//My
#include "TradingCatCommon/transmitdata.h"
#include "TradingCatCommon/userconfig.h"
#include "TradingCatCommon/detector.h"

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     The ServerStatusQuery class - запрос статуса сервера
///
class ServerStatusQuery
    : public Query
{
public:
    /*!
        Конструтор. Создает пустой запрос
    */
    ServerStatusQuery();

    /*!
        Конструктор. Создает запрос на основе запрощенного у сервера URL
        @param query - URL запрос на сервер
    */
    explicit ServerStatusQuery(const QUrlQuery& query);

    /*!
        Деструктор
    */
    ~ServerStatusQuery() override = default;

    //QUrlQuery query() const override;  //Not used.

};

///////////////////////////////////////////////////////////////////////////////
///     The ServerStatus class - данные о состоянии сервера
///
class ServerStatusAnswer final
{
public:
    /*!
        Конструктор. Создает пустой ответ.
    */
    ServerStatusAnswer() = default;

    /*!
        Основной конструтор. Использутся на стороне сервера
        @param serverName - название сервера
        @param serverVersion - версия сервера
        @param serverTime - время на сервере
        @param upTime - время аптайма в секундах
        @param usersOnline - список пользоватлелей онлайн
    */
    ServerStatusAnswer(const QString& serverName, const QString& serverVersion, const QDateTime& serverTime, const qint64 upTime, const QStringList& usersOnline);

    /*!
        Конструтор используется на стороне клиента
        @param json - JSON представление пакета
    */
    explicit ServerStatusAnswer(const QJsonValue& json);

    /*!
        Преобразует данные ответа в JSON объект
        @return JSON объект
    */
    QJsonObject toJson() const;

    /*!
        Время на сервере
        @return время на сервере
    */
    const QDateTime& serverTime() const noexcept;

    /*!
        Возвращает время аптайма сервера в секундах
        @return время аптайма сервера
    */
    qint64 upTime() const noexcept;

    /*!
        Возвращает название сервера
        @return название сервера
    */
    const QString& serverName() const noexcept;

    /*!
        Возвращает версию сервера
        @return версия сервера
    */
    const QString& serverVersion() const noexcept;

    /*!
        Возвращает список пользователей онлайн в формате ИД_ПОЛЬЗОВАТЕЛЯ(ИД_СЕССИИ)
        @return список пользователей онлайн
    */
    const QStringList& usersOnline() const noexcept;

    /*!
        Возвращает true если ответ распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;  ///< Описание ошибки парсинга

    //data
    QDateTime _serverTime = QDateTime::currentDateTime(); ///< локальне время сервера
    qint64 _upTime = 0;         ///< время аптайма сервера
    QString _serverName;        ///< название сервера
    QString _serverVersion;     ///< версия сервера
    QStringList _usersOnline;   ///< список пользователей онлайн

};

///////////////////////////////////////////////////////////////////////////////
///     The LoginQuery class - запрос запрос логина
///
class LoginQuery
    : public Query
{
public:
    LoginQuery();
    explicit LoginQuery(const QUrlQuery& query);
    LoginQuery(const QString& user, const QString& password);

    ~LoginQuery() override = default;

    QUrlQuery query() const override;

    const QString& user() const noexcept;
    const QString& password() const noexcept;

    /*!
        Возвращает true если запрос распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;   ///< Описание ошибки парсинга

    QString _user;
    QString _password;

};

///////////////////////////////////////////////////////////////////////////////
///     The LoginAnswer class - класс ответа на запрос login
///
class LoginAnswer final
{
public:
    LoginAnswer() = default;
    LoginAnswer(qint64 sessionId, const TradingCatCommon::UserConfig& config, const QString& message);
    explicit LoginAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    /*!
        Возвращает ИД сессии пользователя
        @return ИД сессии пользователя
     */
    qint64 sessionId() const noexcept;
    const QString& message() const noexcept;
    const TradingCatCommon::UserConfig& config() const noexcept;

    /*!
        Возвращает true если ответ распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;   ///< Описание ошибки парсинга

    qint64 _sessionId = 0;  ///< ИД сессии пользователя
    TradingCatCommon::UserConfig _config;
    QString _message;

};

///////////////////////////////////////////////////////////////////////////////
///     The LogoutQuery class - запрос запрос логина
///
class LogoutQuery
    : public Query
{
public:
    LogoutQuery();
    explicit LogoutQuery(const QUrlQuery& query);
    LogoutQuery(qint64 sessionId);

    ~LogoutQuery() override = default;

    QUrlQuery query() const override;

    /*!
        Возвращает ИД сессии пользователя
        @return ИД сессии пользователя
     */
    qint64 sessionId() const noexcept;

    /*!
        Возвращает true если запрос распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;   ///< Описание ошибки парсинга

    qint64 _sessionId = 0;  ///< ИД сессии пользователя

};

///////////////////////////////////////////////////////////////////////////////
///     The LogoutAnswer class - класс ответа на запрос login
///
class LogoutAnswer final
{
public:
    LogoutAnswer() = default;
    LogoutAnswer(const QString& message);
    explicit LogoutAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const QString& message() const noexcept;

    /*!
        Возвращает true если ответ распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString; ///< Описание ошибки парсинга

    QString _message;

};

///////////////////////////////////////////////////////////////////////////////
///     The ConfigQuery class - запрос применения новой конфигурации клиента
///
class ConfigQuery
    : public Query
{
public:
    ConfigQuery();
    explicit ConfigQuery(const QUrlQuery& query);
    ConfigQuery(qint64 sessionId, const TradingCatCommon::UserConfig& config);

    ~ConfigQuery() override = default;

    QUrlQuery query() const override;

    /*!
        Возвращает ИД сессии пользователя
        @return ИД сессии пользователя
    */
    qint64 sessionId() const noexcept;
    const TradingCatCommon::UserConfig& config() const noexcept;

    /*!
        Возвращает true если запрос распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;   ///< Описание ошибки парсинга

    qint64 _sessionId = 0;  ///< ИД сессии пользователя
    TradingCatCommon::UserConfig _config;

};

///////////////////////////////////////////////////////////////////////////////
///     The ConfigAnswer class - класс ответа на запрос ConfigQuery
///
class ConfigAnswer final
{
public:
    ConfigAnswer() = default;
    ConfigAnswer(const QString& message);
    explicit ConfigAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const QString& message() const noexcept;

    /*!
        Возвращает true если ответ распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString; ///< Описание ошибки парсинга

    QString _message;

};

///////////////////////////////////////////////////////////////////////////////
///     The StockExchangesQuery class - запрос списка доступных бирж
///
class StockExchangesQuery
    : public Query
{
public:
    StockExchangesQuery();
    explicit StockExchangesQuery(const QUrlQuery& query);
    explicit StockExchangesQuery(qint64 sessionId);

    ~StockExchangesQuery() override = default;

    QUrlQuery query() const override;

    /*!
        Возвращает ИД сессии пользователя
        @return ИД сессии пользователя
    */
    qint64 sessionId() const noexcept;

    /*!
        Возвращает true если запрос распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;   ///< Описание ошибки парсинга

    qint64 _sessionId = 0;  ///< ИД сессии пользователя

};

///////////////////////////////////////////////////////////////////////////////
///     The  StockExchangesAnswer class - класс ответа на запрос  StockExchangesQuery
///
class  StockExchangesAnswer final
{
public:
    StockExchangesAnswer() = default;
    explicit StockExchangesAnswer(const TradingCatCommon::StockExchangesIDList& stockExchangeIdList, const QString& message);
    explicit StockExchangesAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const TradingCatCommon::StockExchangesIDList& stockExchangeIdList() const noexcept;
    const QString& message() const noexcept;

    /*!
        Возвращает true если ответ распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString; ///< Описание ошибки парсинга

    TradingCatCommon::StockExchangesIDList _stockExchangeIdList;

    QString _message;

};

///////////////////////////////////////////////////////////////////////////////
///     The DetectQuery class - запрос списка отфильтрованных свечей
///
class DetectQuery
    : public Query
{
public:
    DetectQuery();
    explicit DetectQuery(const QUrlQuery& query);
    explicit DetectQuery(qint64 sessionId);

    ~DetectQuery() override = default;

    QUrlQuery query() const override;

    /*!
        Возвращает ИД сессии пользователя
        @return ИД сессии пользователя
    */
    qint64 sessionId() const noexcept;

    /*!
        Возвращает true если запрос распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;   ///< Описание ошибки парсинга

    qint64 _sessionId = 0;  ///< ИД сессии пользователя

};

///////////////////////////////////////////////////////////////////////////////
///     The  StockExchangesAnswer class - класс ответа на запрос  StockExchangesQuery
///
class  DetectAnswer final
{
public:
    DetectAnswer() = default;
    explicit DetectAnswer(const TradingCatCommon::Detector::KLinesDetectedList& klinesDetectedList, const QString& message);
    explicit DetectAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const TradingCatCommon::Detector::KLinesDetectedList& klinesDetectedList() const noexcept;
    const QString& message() const noexcept;

    /*!
        Возвращает true если ответ распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;   ///< Описание ошибки парсинга

    TradingCatCommon::Detector::KLinesDetectedList _klinesDetectedList;

    QString _message;

};

///////////////////////////////////////////////////////////////////////////////
///     The KLinesIDList class - запрос списка свичей поддерживаемых биржей
///
class KLinesIDListQuery
    : public Query
{
public:
    KLinesIDListQuery();
    explicit KLinesIDListQuery(const QUrlQuery& query);
    explicit KLinesIDListQuery(qint64 sessionId, const TradingCatCommon::StockExchangeID& stockExchangeId);

    ~KLinesIDListQuery() override = default;

    QUrlQuery query() const override;

    /*!
        Возвращает ИД сессии пользователя
        @return ИД сессии пользователя
    */
    qint64 sessionId() const noexcept;
    const TradingCatCommon::StockExchangeID& stockExchangeId() const noexcept;

    /*!
        Возвращает true если запрос распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;   ///< Описание ошибки парсинга

    qint64 _sessionId = 0;  ///< ИД сессии пользователя
    TradingCatCommon::StockExchangeID _stockExchangeId;

};

///////////////////////////////////////////////////////////////////////////////
///     The  KLinesIDListAnswer class - класс ответа на запрос KLinesIDListQuery
///
class  KLinesIDListAnswer final
{
public:
    KLinesIDListAnswer() = default;
    explicit KLinesIDListAnswer(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesIDList& klinesIdList, const QString& message);
    explicit KLinesIDListAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const TradingCatCommon::StockExchangeID& stockExchangeId() const noexcept;
    const TradingCatCommon::PKLinesIDList& klinesIdList() const noexcept;
    const QString& message() const noexcept;

    /*!
        Возвращает true если ответ распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    QString _errorString;   ///< Описание ошибки парсинга

    TradingCatCommon::StockExchangeID _stockExchangeId;
    TradingCatCommon::PKLinesIDList _klinesIdList;
    QString _message;

};

///////////////////////////////////////////////////////////////////////////////////
///     The  KLinesDataQuery class - запрос списка свечей за определенный период
///
class KLinesDataQuery
    : public Query
{
public:
    KLinesDataQuery();
    explicit KLinesDataQuery(const QUrlQuery& query);
    KLinesDataQuery(qint64 sessionId,
                    TradingCatCommon::StockExchangeID stockExchangeId,
                    TradingCatCommon::KLineID klineId,
                    qint64 start,
                    qint64 end);

    ~KLinesDataQuery() override = default;

    QUrlQuery query() const override;

    /*!
        Возвращает ИД сессии пользователя
        @return ИД сессии пользователя
    */
    qint64 sessionId() const noexcept;
    const TradingCatCommon::StockExchangeID& stockExchangeId() const noexcept;
    const TradingCatCommon::KLineID& klineId() const noexcept;
    qint64 start() const noexcept;
    qint64 end() const noexcept;

    /*!
        Возвращает true если запрос распарсился с ошибкой. Текстовое описание ошибки можно получить методом errorString()
        @return true - есть ошибка, false - все ок
    */
    bool isError() const noexcept;

    /*!
        Возвращает текстовое описание последней ошибки. Если все ок - возвращает пустую строку
        @return текстовое описание последней ошибки
    */
    const QString& errorString() const noexcept;

private:
    Q_DISABLE_COPY_MOVE(KLinesDataQuery);

private:
    QString _errorString;   ///< Описание ошибки парсинга

    qint64 _sessionId = 0;  ///< ИД сессии пользователя
    TradingCatCommon::StockExchangeID _stockExchangeId;
    TradingCatCommon::KLineID _klineId;
    qint64 _start = 0;
    qint64 _end = 0;

};

} //namespace TradingCatCommon
