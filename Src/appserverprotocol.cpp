//My
#include <Common/parser.h>

#include "TradingCatCommon/appserverprotocol.h"

using namespace TradingCatCommon;
using namespace Common;

Q_GLOBAL_STATIC(const qsizetype, MAX_USER_LENGTH, 20);
Q_GLOBAL_STATIC(const qsizetype, MAX_PASSWORD_LENGTH, 20);

///////////////////////////////////////////////////////////////////////////////
///     The StockExchangesQuery class - запрос списка поддерживаемых сервером бирж
///

ServerStatusQuery::ServerStatusQuery()
    : Query(PackageType::SERVER_STATUS, "/status")
{
}

ServerStatusQuery::ServerStatusQuery(const QUrlQuery &query)
    : ServerStatusQuery()
{
    Q_UNUSED(query);
}

ServerStatusQuery::~ServerStatusQuery()
{
}

///////////////////////////////////////////////////////////////////////////////
///     ServerStatus::serverTime
///
ServerStatusAnswer::ServerStatusAnswer(const QString& serverName, const QString& serverVersion, const QDateTime &serverTime, const qint64 upTime, const QStringList& usersOnline)
    : _serverTime(serverTime)
    , _upTime(upTime)
    , _serverName(serverName)
    , _serverVersion(serverVersion)
    , _usersOnline(usersOnline)
{
}

const QDateTime &ServerStatusAnswer::serverTime() const noexcept
{
    return _serverTime;
}

qint64 ServerStatusAnswer::upTime() const noexcept
{
    return _upTime;
}

const QString &ServerStatusAnswer::serverName() const noexcept
{
    return _serverName;
}

const QString &ServerStatusAnswer::serverVersion() const noexcept
{
    return _serverVersion;
}

bool ServerStatusAnswer::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &ServerStatusAnswer::errorString() const noexcept
{
    return _errorString;
}

ServerStatusAnswer::ServerStatusAnswer(const QJsonValue& json)
{
    try
    {
        const auto dataJson = JSONReadMap(json, "Root/Data");

        _serverTime = JSONReadMapDateTime(dataJson, "ServerTime", "Root/Data/ServerTime").value_or(QDateTime::currentDateTime());
        _upTime = JSONReadMapNumber<qint64>(dataJson, "Uptime", "Root/Data/UpTime").value_or(0);
        _serverName = JSONReadMapString(dataJson, "ServerName", "Root/Data/ServerName").value_or("No name");
        _serverVersion = JSONReadMapString(dataJson, "ServerVersion", "Root/Data/VersionName").value_or("undefined");
        _usersOnline = JSONReadMapString(dataJson, "UsersOnline", "Root/Data/UsersOnline").value_or("null").split(',');
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
        _serverTime = QDateTime();
        _upTime = 0;
        _serverName.clear();
        _serverVersion.clear();
    }
}

QJsonObject ServerStatusAnswer::toJson() const
{
    QJsonObject statusJson;

    statusJson.insert("ServerTime", QDateTime::currentDateTime().toString(DATETIME_FORMAT));
    statusJson.insert("Uptime", _upTime);
    statusJson.insert("ServerName", _serverName);
    statusJson.insert("ServerVersion", _serverVersion);
    statusJson.insert("UsersOnline", _usersOnline.join(','));

    return statusJson;
}


///////////////////////////////////////////////////////////////////////////////
///     The LoginUserQuery class - запрос запрос логина
///
LoginQuery::LoginQuery()
    : Query(PackageType::LOGIN, "/users/login")
{
}

LoginQuery::LoginQuery(const QUrlQuery &query)
    : LoginQuery()
{
    try
    {
        _user = query.queryItemValue("user");
        if (_user.isEmpty() || _user.length() > *MAX_USER_LENGTH)
        {
            throw ParseException(QString("Value of key 'user' cannot be empty or longer than %1 characters").arg(*MAX_PASSWORD_LENGTH));
        }

        _password = query.queryItemValue("password");
        if (_password.isEmpty() || _password.length() > *MAX_PASSWORD_LENGTH)
        {
            throw ParseException(QString("Value of key 'password' cannot be empty or longer than %1 characters").arg(*MAX_PASSWORD_LENGTH));
        }
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
        _user.clear();
        _password.clear();
    }
}

LoginQuery::LoginQuery(const QString &user, const QString &password)
    : LoginQuery()
{
    Q_ASSERT(!user.isEmpty());

    _user = user;
    _password = password;
}

QUrlQuery LoginQuery::query() const
{
    QUrlQuery query;

    query.addQueryItem("user", _user);
    query.addQueryItem("password", _password);

    return query;
}

const QString &LoginQuery::user() const noexcept
{
    return _user;
}

const QString &LoginQuery::password() const noexcept
{
    return _password;
}

bool LoginQuery::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &LoginQuery::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The LoginAnswer class - класс ответа на запрос login
///
LoginAnswer::LoginAnswer(qint64 sessionId, const TradingCatCommon::UserConfig& config, const QString& message)
    : _sessionId(sessionId)
    , _config(config)
    , _message(message) 
{
}

LoginAnswer::LoginAnswer(const QJsonValue &json)
{
    try
    {
        const auto dataJson = JSONReadMap(json, "Root/Data");

        _sessionId = JSONReadMapNumber<qint64>(dataJson, "SessionId", "Root/Data/SessionId").value_or(0);
        _message = JSONReadMapString(dataJson, "Msg", "Root/Data/Msg").value_or("");

        const auto configBase64Result = QByteArray::fromBase64Encoding(JSONReadMapString(dataJson, "Config", "Root/Data/Config").value_or("").toUtf8(), QByteArray::Base64UrlEncoding);
        if (configBase64Result.decodingStatus != QByteArray::Base64DecodingStatus::Ok)
        {
            throw ParseException(QString("Incorrect format of [Root/Data/Config]: error decode from base64"));
        }

        _config = UserConfig(configBase64Result.decoded);
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();

    }
}

QJsonObject LoginAnswer::toJson() const
{
    QJsonObject dataJson;

    dataJson.insert("SessionId", QJsonValue(_sessionId));
    dataJson.insert("Msg", _message);
    dataJson.insert("Config", QString(_config.toJson().toUtf8().toBase64(QByteArray::Base64UrlEncoding)));

    return dataJson;
}

qint64 LoginAnswer::sessionId() const noexcept
{
    return _sessionId;
}

const QString &LoginAnswer::message() const noexcept
{
    return _message;
}

const UserConfig &LoginAnswer::config() const noexcept
{
    return _config;
}

bool LoginAnswer::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &LoginAnswer::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The LogoutQuery class - запрос запрос логина
///
LogoutQuery::LogoutQuery()
    : Query(PackageType::LOGOUT, "/users/logout")
{
}

LogoutQuery::LogoutQuery(const QUrlQuery &query)
    : LogoutQuery()
{
    try
    {
        bool ok = false;
        _sessionId = query.queryItemValue("sessionId").toLong(&ok);

        if (! ok || _sessionId == 0)
        {
            throw ParseException("Value of key 'sessionId' must be non zero number");
        }
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
        _sessionId = 0;
    }
}

LogoutQuery::LogoutQuery(qint64 sessionId)
    : LogoutQuery()
{
    Q_ASSERT(sessionId != 0);

    _sessionId = sessionId;
}

QUrlQuery LogoutQuery::query() const
{
    QUrlQuery query;

    query.addQueryItem("sessionId", QString::number(_sessionId));

    return query;
}

qint64 LogoutQuery::sessionId() const noexcept
{
    return _sessionId;
}

bool LogoutQuery::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &LogoutQuery::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The LogoutAnswer class - класс ответа на запрос login
///
LogoutAnswer::LogoutAnswer(const QString &message)
    : _message(message)
{

}

LogoutAnswer::LogoutAnswer(const QJsonValue &json)
{
    try
    {
        const auto dataJson = JSONReadMap(json, "Root/Data");

        _message = JSONReadMapString(dataJson, "Msg", "Root/Data/Msg").value_or("");
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
        _message.clear();
    }
}

QJsonObject LogoutAnswer::toJson() const
{
    QJsonObject dataJson;

    dataJson.insert("Msg", _message);

    return dataJson;
}

const QString &LogoutAnswer::message() const noexcept
{
    return _message;
}

bool LogoutAnswer::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &LogoutAnswer::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The LogoutQuery class - запрос запрос логина
///
ConfigQuery::ConfigQuery()
    : Query(PackageType::CONFIG, "/users/config")
{
}

ConfigQuery::ConfigQuery(const QUrlQuery &query)
    : ConfigQuery()
{
    try
    {
        bool ok = false;
        _sessionId = query.queryItemValue("sessionId").toLong(&ok);
        if (!ok || _sessionId == 0)
        {
            throw ParseException("Value of key 'sessionId' must be non zero number");
        }

        if (!query.hasQueryItem("config"))
        {
            throw ParseException(QString("Value of key 'config' cannot be undefined"));
        }

        const auto configBase64Result = QByteArray::fromBase64Encoding(query.queryItemValue("config").toUtf8(), QByteArray::Base64UrlEncoding);
        if (configBase64Result.decodingStatus != QByteArray::Base64DecodingStatus::Ok)
        {
            throw ParseException(QString("Incorrect format of key 'config': error decode from base64"));
        }

        UserConfig config(configBase64Result.decoded);

        if(config.isError())
        {
            throw ParseException(QString("Incorrect format of key 'config': %1").arg(config.errorString()));
        }

        _config = config;
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
        _sessionId = 0;
        _config = UserConfig();
    }
}

ConfigQuery::ConfigQuery(qint64 sessionId, const UserConfig &config)
    : ConfigQuery()
{
    Q_ASSERT(sessionId != 0);

    _sessionId = sessionId;
    _config = config;
}

QUrlQuery ConfigQuery::query() const
{
    QUrlQuery query;

    query.addQueryItem("sessionId", QString::number(_sessionId));
    query.addQueryItem("config", _config.toJson().toUtf8().toBase64(QByteArray::Base64UrlEncoding));

    return query;
}

qint64 ConfigQuery::sessionId() const noexcept
{
    return _sessionId;
}

bool ConfigQuery::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &ConfigQuery::errorString() const noexcept
{
    return _errorString;
}

const UserConfig &ConfigQuery::config() const noexcept
{
    return _config;
}

///////////////////////////////////////////////////////////////////////////////
///     The ConfigAnswer class - класс ответа на запрос ConfigQuery
///
ConfigAnswer::ConfigAnswer(const QString &message)
    : _message(message)
{
}

ConfigAnswer::ConfigAnswer(const QJsonValue &json)
{
    try
    {
        const auto dataJson = JSONReadMap(json, "Root/Data");

        _message = JSONReadMapString(dataJson, "Msg", "Root/Data/Msg").value_or("");
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
        _message.clear();
    }
}

QJsonObject ConfigAnswer::toJson() const
{
    QJsonObject dataJson;

    dataJson.insert("Msg", _message);

    return dataJson;
}

const QString &ConfigAnswer::message() const noexcept
{
    return _message;
}

bool ConfigAnswer::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &ConfigAnswer::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The StockExchangesQuery class - запрос списка доступных бирж
///
StockExchangesQuery::StockExchangesQuery()
    : Query(PackageType::STOCKEXCHANGES, "/data/stockexchanges")
{
}

StockExchangesQuery::StockExchangesQuery(const QUrlQuery &query)
    : StockExchangesQuery()
{
    try
    {
        bool ok = false;
        _sessionId = query.queryItemValue("sessionId").toLong(&ok);
        if (! ok || _sessionId == 0)
        {
            throw ParseException("Value of key 'sessionId' must be non zero number");
        }
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
        _sessionId = 0;
    }
}

StockExchangesQuery::StockExchangesQuery(qint64 sessionId)
    : StockExchangesQuery()
{
    Q_ASSERT(sessionId != 0);

    _sessionId = sessionId;
}

QUrlQuery StockExchangesQuery::query() const
{
    QUrlQuery query;

    query.addQueryItem("sessionId", QString::number(_sessionId));

    return query;
}

qint64 StockExchangesQuery::sessionId() const noexcept
{
    return _sessionId;
}

bool StockExchangesQuery::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &StockExchangesQuery::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The  StockExchangesAnswer class - класс ответа на запрос  StockExchangesQuery
///
StockExchangesAnswer::StockExchangesAnswer(const StockExchangesIDList &stockExchangeIdList, const QString& message)
    : _stockExchangeIdList(stockExchangeIdList)
    , _message(message)
{
}

StockExchangesAnswer::StockExchangesAnswer(const QJsonValue &json)
{
    try
    {
        const auto dataJson = JSONReadMap(json, "Root/Data");

        _message = JSONReadMapString(dataJson, "Msg", "Root/Data/Msg").value_or("");

        const auto stockExchangeIdArrayJson = JSONReadMapToArray(dataJson, "StockExchanges", "Root/Data/StockExchanges");
        StockExchangesIDArrayJson stockExchangesIDJson(stockExchangeIdArrayJson);

        if (stockExchangesIDJson.isError())
        {
            _errorString =  stockExchangesIDJson.errorString();

            return;
        }

        _stockExchangeIdList = stockExchangesIDJson.stockExchagesIDList();
    }
    catch (const ParseException& err)
    {
        _message.clear();
        _stockExchangeIdList.clear();

        _errorString = err.what();
    }
}

QJsonObject StockExchangesAnswer::toJson() const
{
    QJsonObject result;

    result.insert("Msg", _message);
    result.insert("StockExchanges", StockExchangesIDArrayJson(_stockExchangeIdList).toJson());

    return result;
}

const QString &StockExchangesAnswer::message() const noexcept
{
    return _message;
}

const StockExchangesIDList &StockExchangesAnswer::stockExchangeIdList() const noexcept
{
    return _stockExchangeIdList;
}

bool StockExchangesAnswer::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &StockExchangesAnswer::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The DetectQuery class - запрос списка отфильтрованных свечей
///
DetectQuery::DetectQuery()
    : Query(PackageType::DETECT, "/data/detect")
{
}

DetectQuery::DetectQuery(const QUrlQuery &query)
    : DetectQuery()
{
    try
    {
        bool ok = false;
        _sessionId = query.queryItemValue("sessionId").toLong(&ok);
        if (!ok || _sessionId == 0)
        {
            throw ParseException("Value of key 'sessionId' must be non zero number");
        }
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
        _sessionId = 0;
    }
}

DetectQuery::DetectQuery(qint64 sessionId)
    : DetectQuery()
{
    Q_ASSERT(sessionId != 0);

    _sessionId = sessionId;
}

QUrlQuery DetectQuery::query() const
{
    QUrlQuery query;

    query.addQueryItem("sessionId", QString::number(_sessionId));

    return query;
}

qint64 DetectQuery::sessionId() const noexcept
{
    return _sessionId;
}

bool DetectQuery::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &DetectQuery::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The DetectAnswer class - класс ответа на запрос  DetectQuery
///
DetectAnswer::DetectAnswer(const Detector::KLinesDetectedList &klinesDetectedList, const QString &message)
    : _klinesDetectedList(klinesDetectedList)
    , _message(message)
{
}

DetectAnswer::DetectAnswer(const QJsonValue &json)
{
    try
    {
        const auto objJson = JSONReadMap(json, "Root/Data");
        _klinesDetectedList.isFull = JSONReadMapBool(objJson, "IsFull", "Root/Data/IsFull").value_or(false);

        const auto detectListJson = JSONReadMapToArray(objJson, "Detect", "Root/Data/Detect");

        for (const auto& detectJsonValue: detectListJson)
        {
            const auto detectJson = JSONReadMap(detectJsonValue, "Root/Data/Detect");

            auto detect = std::make_shared<TradingCatCommon::Detector::KLineDetectData>();

            {
                StockExchangeIDJson stockExchangeId(JSONReadMapToMap(detectJson, "StockExchangeID", "Root/Data/Detect/[]/StockExchnageId"));

                if (stockExchangeId.isError())
                {
                    throw ParseException(QString("Error parsing Root/Data/Detect/[]/StockExchangeID: %1").arg(stockExchangeId.errorString()));
                }

                detect->stockExchangeId = stockExchangeId.stockExchangeId();

                if (detect->stockExchangeId.isEmpty())
                {
                    throw ParseException("Value of Root/Data/Detect/[]/StockExchangeID cannot be empty");
                }
            }

            {
                const auto o_delta = JSONReadMapNumber<double>(detectJson, "Delta", "Root/Data/Detect/[]/Delta", 0.0f);
                if (!o_delta.has_value())
                {
                    throw ParseException("Value of Root/Data/Detect/[]/Delta cannot be undefined");
                }

                detect->delta = o_delta.value();
            }

            {
                const auto o_volume = JSONReadMapNumber<double>(detectJson, "Volume", "Root/Data/Detect/[]/Volume", 0.0f);
                if (!o_volume.has_value())
                {
                    throw ParseException("Value of Root/Data/Detect/[]/Volume cannot be undefined");
                }
                detect->volume = o_volume.value();
            }

            {
                const auto o_msg = JSONReadMapString(detectJson, "Msg", "Root/Data/Detect/[]/Msg");
                if (!o_msg.has_value())
                {
                    throw ParseException("Value of Root/Data/Detect/[]/Msg cannot be undefined");
                }

                detect->msg = o_msg.value();
            }

            {
                const auto o_filterActive = JSONReadMapNumber<quint32>(detectJson, "FilterActivate", "Root/Data/Detect/[]/FilterActivate");
                if (!o_filterActive.has_value())
                {
                    throw ParseException("Value of Root/Data/Detect/[]/FilterActivate cannot be undefined");
                }

                detect->filterActivate.fromInt(o_filterActive.value());
            }

            {
                const auto historyArrayJson = JSONReadMapToArray(detectJson, "History", "Root/Data/Detect/[]/History");
                KLinesArrayJson history(historyArrayJson);

                if (history.isError())
                {
                    throw ParseException(QString("Invalid value Root/Data/Detect/[]/History: %1").arg(history.errorString()));
                }

                detect->history = history.klinesList();

                if (detect->history->empty())
                {
                     throw ParseException(QString("Value Root/Data/Detect/[]/History cannot be empty"));
                }
            }

            {
                const auto reviewHistoryArrayJson = JSONReadMapToArray(detectJson, "ReviewHistory", "Root/Data/Detect/[]/ReviewHistory");
                KLinesArrayJson reviewHistory(reviewHistoryArrayJson);

                if (reviewHistory.isError())
                {
                    throw ParseException(QString("Invalid value Root/Data/Detect/[]/ReviewHistory: %1").arg(reviewHistory.errorString()));
                }

                detect->reviewHistory = reviewHistory.klinesList();

                if (detect->reviewHistory->empty())
                {
                    throw ParseException(QString("Value Root/Data/Detect/[]/ReviewHistory cannot be empty"));
                }
            }

            _klinesDetectedList.detected.emplace_back(std::move(detect));
        }

    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
    }
}

QJsonObject DetectAnswer::toJson() const
{
    QJsonObject resultJson;

    resultJson.insert("IsFull", _klinesDetectedList.isFull);

    QJsonArray detectedListJson;
    for(const auto& detected: _klinesDetectedList.detected)
    {
        Q_CHECK_PTR(detected->history);
        Q_CHECK_PTR(detected->reviewHistory);

        Q_ASSERT(!detected->stockExchangeId.isEmpty());
        Q_ASSERT(!detected->history->empty());
        Q_ASSERT(!detected->reviewHistory->empty());

        QJsonObject detectJson;
        detectJson.insert("Delta", detected->delta);
        detectJson.insert("Volume", detected->volume);
        detectJson.insert("FilterActivate", QJsonValue(static_cast<int>(detected->filterActivate.toInt())));
        detectJson.insert("Msg", detected->msg);
        detectJson.insert("StockExchangeID", StockExchangeIDJson(detected->stockExchangeId).toJson());
        detectJson.insert("History", KLinesArrayJson(detected->history).toJson());
        detectJson.insert("ReviewHistory", KLinesArrayJson(detected->reviewHistory).toJson());

        detectedListJson.push_back(detectJson);
    }

    resultJson.insert("Detect", detectedListJson);

    return resultJson;
}

const Detector::KLinesDetectedList &DetectAnswer::klinesDetectedList() const noexcept
{
    return _klinesDetectedList;
}

const QString &DetectAnswer::message() const noexcept
{
    return _message;
}

bool DetectAnswer::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &DetectAnswer::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The KLinesIDList class - запрос списка свичей поддерживаемых биржей
///
KLinesIDListQuery::KLinesIDListQuery()
    : Query(PackageType::KLINESIDLIST, "/data/klines")
{
}

KLinesIDListQuery::KLinesIDListQuery(const QUrlQuery &query)
    : KLinesIDListQuery()
{
    try
    {
        bool ok = false;
        _sessionId = query.queryItemValue("sessionId").toLong(&ok);
        if (! ok || _sessionId == 0)
        {
            throw ParseException("Value of key 'sessionId' must be non zero number");
        }

        const auto stockExhangeIDStr = query.queryItemValue("stockExchangeId");
        if (stockExhangeIDStr.isEmpty())
        {
            throw ParseException("Value of key 'stockExchangeId' cannot be empty");
        }

        _stockExchangeId = StockExchangeID(stockExhangeIDStr);
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
        _sessionId = 0;
        _stockExchangeId = StockExchangeID();
    }
}

KLinesIDListQuery::KLinesIDListQuery(qint64 sessionId, const StockExchangeID &stockExchangeId)
    : KLinesIDListQuery()
{
    Q_ASSERT(sessionId != 0);
    Q_ASSERT(!stockExchangeId.isEmpty());

    _sessionId = sessionId;
    _stockExchangeId = stockExchangeId;
}

QUrlQuery KLinesIDListQuery::query() const
{
    QUrlQuery query;

    query.addQueryItem("sessionId", QString::number(_sessionId));
    query.addQueryItem("stockExchangeId", _stockExchangeId.name);

    return query;
}

qint64 KLinesIDListQuery::sessionId() const noexcept
{
    return _sessionId;
}

bool KLinesIDListQuery::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &KLinesIDListQuery::errorString() const noexcept
{
    return _errorString;
}

const StockExchangeID &KLinesIDListQuery::stockExchangeId() const noexcept
{
    return _stockExchangeId;
}

///////////////////////////////////////////////////////////////////////////////
///     The  KLinesIDListAnswer class - класс ответа на запрос KLinesIDListQuery
///
KLinesIDListAnswer::KLinesIDListAnswer(const StockExchangeID &stockExchangeId, const PKLinesIDList& klinesIdList, const QString& message)
    : _stockExchangeId(stockExchangeId)
    , _klinesIdList(klinesIdList)
    , _message(message)
{
    Q_CHECK_PTR(_klinesIdList);
    Q_ASSERT(!stockExchangeId.isEmpty());
}

KLinesIDListAnswer::KLinesIDListAnswer(const QJsonValue &json)
{
    _klinesIdList = std::make_shared<KLinesIDList>();
    try
    {
        const auto objJson = JSONReadMap(json, "Root/Data");

        const auto stockExchangeIdJson = JSONReadMapToMap(objJson, "StockExchangeID", "Root/Data/StockExchangeId");
        StockExchangeIDJson stockExchangeId(stockExchangeIdJson);

        if (stockExchangeId.isError() || stockExchangeId.stockExchangeId().isEmpty())
        {
            throw ParseException(QString("Error parsing Root/Data/StockExchangeID: %1").arg(stockExchangeId.errorString()));
        }

        _stockExchangeId = stockExchangeId.stockExchangeId();
        if (_stockExchangeId.isEmpty())
        {
            throw ParseException(QString("Error parsing Root/Data/StockExchangeID: stock exchange ID cannot be empty"));
        }

        const auto klinesIdListJson = JSONReadMapToArray(objJson, "KLinesIDList", "Root/Data/KLinesIdList");

        for (const auto& klineIdJson: klinesIdListJson)
        {
            const auto klineId = JSONReadMap(klineIdJson, "Root/Data/KLinesIdList/[]");
            _klinesIdList->insert(KLineIDJson(klineId).klineId());
        }

        _message = JSONReadMapString(objJson, "Msg", "Root/Data/Msg").value_or("");

    }
    catch (const ParseException& err)
    {
        _errorString = err.what();

        _stockExchangeId = StockExchangeID();
        _klinesIdList->clear();

    }
}

QJsonObject KLinesIDListAnswer::toJson() const
{
    QJsonArray klineIdListJson;
    for (const auto& klineId: *_klinesIdList)
    {
        klineIdListJson.push_back(KLineIDJson(klineId).toJson());
    }

    QJsonObject resultJson;
    resultJson.insert("StockExchangeID", StockExchangeIDJson(_stockExchangeId).toJson());
    resultJson.insert("KLinesIDList", klineIdListJson);
    resultJson.insert("Msg", _message);

    return resultJson;
}

const StockExchangeID &KLinesIDListAnswer::stockExchangeId() const noexcept
{
    return _stockExchangeId;
}

const PKLinesIDList &KLinesIDListAnswer::klinesIdList() const noexcept
{
    return _klinesIdList;
}

const QString &KLinesIDListAnswer::message() const noexcept
{
    return _message;
}

bool KLinesIDListAnswer::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &KLinesIDListAnswer::errorString() const noexcept
{
    return _errorString;
}
