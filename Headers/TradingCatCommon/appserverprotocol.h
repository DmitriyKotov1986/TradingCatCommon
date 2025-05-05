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
        Конструтор
    */
    ServerStatusQuery();
    explicit ServerStatusQuery(const QUrlQuery& query);

    /*!
        Деструктор
    */
    ~ServerStatusQuery();

    //QUrlQuery query() const override;  //Not used.

};

///////////////////////////////////////////////////////////////////////////////
///     The ServerStatus class
///
class ServerStatusAnswer
{
public:
    ServerStatusAnswer() = default;
    ServerStatusAnswer(const QString& serverName, const QString& serverVersion, const QDateTime& serverTime, const qint64 upTime, const QStringList& usersOnline);
    explicit ServerStatusAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const QDateTime& serverTime() const noexcept;
    qint64 upTime() const noexcept;
    const QString& serverName() const noexcept;
    const QString& serverVersion() const noexcept;
    const QStringList& usersOnline() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    QDateTime _serverTime = QDateTime::currentDateTime();
    qint64 _upTime = 0;
    QString _serverName;
    QString _serverVersion;
    QStringList _usersOnline;

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

    QUrlQuery query() const override;

    const QString& user() const noexcept;
    const QString& password() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    QString _user;
    QString _password;

};

///////////////////////////////////////////////////////////////////////////////
///     The LoginAnswer class - класс ответа на запрос login
///
class LoginAnswer
{
public:
    LoginAnswer() = default;
    LoginAnswer(qint64 sessionId, const TradingCatCommon::UserConfig& config, const QString& message);
    explicit LoginAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    qint64 sessionId() const noexcept;
    const QString& message() const noexcept;
    const TradingCatCommon::UserConfig& config() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    qint64 _sessionId = 0;
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

    QUrlQuery query() const override;

    qint64 sessionId() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    qint64 _sessionId = 0;

};

///////////////////////////////////////////////////////////////////////////////
///     The LogoutAnswer class - класс ответа на запрос login
///
class LogoutAnswer
{
public:
    LogoutAnswer() = default;
    LogoutAnswer(const QString& message);
    explicit LogoutAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const QString& message() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

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

    QUrlQuery query() const override;

    qint64 sessionId() const noexcept;
    const TradingCatCommon::UserConfig& config() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    qint64 _sessionId = 0;
    TradingCatCommon::UserConfig _config;

};

///////////////////////////////////////////////////////////////////////////////
///     The ConfigAnswer class - класс ответа на запрос ConfigQuery
///
class ConfigAnswer
{
public:
    ConfigAnswer() = default;
    ConfigAnswer(const QString& message);
    explicit ConfigAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const QString& message() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

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

    QUrlQuery query() const override;

    qint64 sessionId() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    qint64 _sessionId = 0;

};

///////////////////////////////////////////////////////////////////////////////
///     The  StockExchangesAnswer class - класс ответа на запрос  StockExchangesQuery
///
class  StockExchangesAnswer
{
public:
    StockExchangesAnswer() = default;
    explicit StockExchangesAnswer(const TradingCatCommon::StockExchangesIDList& stockExchangeIdList, const QString& message);
    explicit StockExchangesAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const TradingCatCommon::StockExchangesIDList& stockExchangeIdList() const noexcept;
    const QString& message() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

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

    QUrlQuery query() const override;

    qint64 sessionId() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    qint64 _sessionId = 0;

};

///////////////////////////////////////////////////////////////////////////////
///     The  StockExchangesAnswer class - класс ответа на запрос  StockExchangesQuery
///
class  DetectAnswer
{
public:
    DetectAnswer() = default;
    explicit DetectAnswer(const TradingCatCommon::Detector::KLinesDetectedList& klinesDetectedList, const QString& message);
    explicit DetectAnswer(const QJsonValue& json);

    QJsonObject toJson() const;

    const TradingCatCommon::Detector::KLinesDetectedList& klinesDetectedList() const noexcept;
    const QString& message() const noexcept;

    bool isError() const noexcept;
    const QString& errorString() const noexcept;

private:
    QString _errorString;

    TradingCatCommon::Detector::KLinesDetectedList _klinesDetectedList;

    QString _message;

};

} //namespace TradingCatCommon
