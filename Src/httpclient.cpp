//Qt
#include <QJsonParseError>

#include "TradingCatCommon/httpclient.h"

using namespace TradingCatCommon;
using namespace Common;

static const quint64 SEND_TIMEOUT = 10u * 1000u; //ms

HTTPClient::HTTPClient(const HTTPClientConfig& config, QObject *parent /* = nullptr */)
    : QObject{parent}
    , _config(config)
{
    Q_ASSERT(_config.isCheck());

    _http = new HTTPSSLQuery();

    connect(_http, SIGNAL(getAnswer(const QByteArray&, quint64)),
            SLOT(getAnswerHTTP(const QByteArray&, quint64)));
    connect(_http, SIGNAL(errorOccurred(QNetworkReply::NetworkError, quint64, const QString&, quint64)),
            SLOT(errorOccurredHTTP(QNetworkReply::NetworkError, quint64, const QString&, quint64)));
    connect(_http, SIGNAL(sendLogMsg(Common::TDBLoger::MSG_CODE, const QString&, quint64)),
            SLOT(sendLogMsgHTTP(Common::TDBLoger::MSG_CODE, const QString&, quint64)));
}

quint64 HTTPClient::getPackage(std::unique_ptr<Query>&& query)
{
    Q_CHECK_PTR(_http);
    Q_CHECK_PTR(query);

    QUrl url(QString("http://%1").arg(_config.address.toString()));
    url.setPort(_config.port);
    url.setPath(QString("/%1").arg(query->path()));

    const auto queryList = query->query();
    if (!queryList.isEmpty())
    {
        url.setQuery(queryList);
    }

    query->trySend();

    const auto id = _http->send(url, HTTPSSLQuery::RequestType::GET);

    if (query->isFirstUse())
    {
        query->setID(id);
    }

    _package.emplace(id, std::move(query));

    return id;
}

void HTTPClient::retryGetPackage(std::unique_ptr<Query>&& query)
{
    QTimer::singleShot(SEND_TIMEOUT, this,
        [query = std::move(query), this]() mutable
        {
            getPackage(std::move(query));
        });
}

HTTPClient::ParseResult HTTPClient::parseStockExchangeList(const QByteArray &answer, const Query* query)
{
    QJsonParseError error;
    const auto jsonDoc = QJsonDocument::fromJson(answer, &error);
    if (error.error != QJsonParseError::NoError)
    {
        return QString("Error JSON parsing: %1").arg(error.errorString());
    }

    Package<StockExchangesIDArrayJson> package(jsonDoc);
    if (package.isError())
    {
        return QString("Error get answer StockExchangeList from Stock exchage server. Error: %1").arg(package.errorString());
    }

    const auto& data = package.data();

    emit stockExchangeList(data.toStockExchagesIDList(), query->id());

    return std::nullopt;
}

HTTPClient::ParseResult HTTPClient::parseKLineList(const QByteArray &answer, const Query* query)
{
    QJsonParseError error;
    const auto jsonDoc = QJsonDocument::fromJson(answer, &error);
    if (error.error != QJsonParseError::NoError)
    {
        return QString("Error JSON parsing: %1").arg(error.errorString());
    }

    Package<KLinesIDArrayJson> package(jsonDoc);
    if (package.isError())
    {
        return QString("Error get answer KLineList from Stock exchage server. Error: %1").arg(package.errorString());
    }

    const auto& data = package.data();
    const auto klinesIdList = data.toKLinesIDList();
    if (klinesIdList.empty())
    {
        return QString("Get empty KLine list");
    }

    const auto p_query =  dynamic_cast<const KLinesListQuery*>(query);

    emit klineList(p_query->stockExchageId(), klinesIdList, p_query->id());

    return std::nullopt;
}

HTTPClient::ParseResult HTTPClient::parseKLineNew(const QByteArray &answer, const Query* query)
{
    QJsonParseError error;
    const auto jsonDoc = QJsonDocument::fromJson(answer, &error);
    if (error.error != QJsonParseError::NoError)
    {
        return QString("Error JSON parsing: %1").arg(error.errorString());
    }

    Package<KLinesNewArrayJson> package(jsonDoc);
    if (package.isError())
    {
        return QString("Error get answer KLineNew from Stock exchage server. Error: %1").arg(package.errorString());
    }

    const auto& data = package.data();

    std::unordered_map<StockExchangeID, PKLinesList> newKLines;
    for (const auto& kline: data.klines())
    {
        const auto& stockExchangeId = kline.stockExchangeId();
        auto it_newKLines = newKLines.find(stockExchangeId);
        if (it_newKLines == newKLines.end())
        {
            it_newKLines = newKLines.emplace(stockExchangeId, std::make_shared<KLinesList>()).first;
        }
        (*it_newKLines->second).push_back(kline.kline());
    }

    _lastGetKLineId = data.lastGetId();

    for (const auto& [stockExchangeId, klinesList]: newKLines)
    {
        emit klineNew(stockExchangeId, klinesList, query->id());
    }

    return std::nullopt;
}

HTTPClient::ParseResult HTTPClient::parseKLineHistory(const QByteArray &answer, const Query* query)
{
    QJsonParseError error;
    const auto jsonDoc = QJsonDocument::fromJson(answer, &error);
    if (error.error != QJsonParseError::NoError)
    {
        return QString("Error JSON parsing: %1").arg(error.errorString());
    }

    Package<KLinesHistoryArrayJson> package(jsonDoc);
    if (package.isError())
    {
        return QString("Error get answer KLineHistory from Stock exchage server. Error: %1").arg(package.errorString());
    }

    const auto& data = package.data();

    std::unordered_map<StockExchangeID, PKLinesList> newKLines;
    for (const auto& kline: data.klines())
    {
        const auto& stockExchangeId = kline.stockExchangeId();
        auto it_newKLines = newKLines.find(stockExchangeId);
        if (it_newKLines == newKLines.end())
        {
            it_newKLines = newKLines.emplace(stockExchangeId, std::make_shared<KLinesList>()).first;
        }
        it_newKLines->second->push_back(kline.kline());
    }

    for (const auto& [stockExchangeId, klinesList]: newKLines)
    {
        emit klineHistory(stockExchangeId, klinesList, query->id());
    }

    return std::nullopt;
}

HTTPClient::ParseResult HTTPClient::parseServerStatus(const QByteArray& answer, const Query* query)
{
    QJsonParseError error;
    const auto jsonDoc = QJsonDocument::fromJson(answer, &error);
    if (error.error != QJsonParseError::NoError)
    {
        return QString("Error JSON parsing: %1").arg(error.errorString());
    }

    Package<ServerStatusJson> package(jsonDoc);
    if (package.isError())
    {
        return QString("Error get answer ServerStatus from Stock exchage server. Error: %1").arg(package.errorString());
    }

    const auto& data = package.data();

    emit serverStatus(data.serverName(), data.serverVersion(), data.serverTime(), data.upTime(), query->id());

    return std::nullopt;
}

void HTTPClient::getAnswerHTTP(const QByteArray& answer, quint64 id)
{
    auto it_package = _package.find(id);
    if (it_package == _package.end())
    {
        Q_ASSERT(false);
    }

    auto& query = it_package->second;
    const auto packageType = query->type();

    ParseResult parseResult;
    switch (packageType)
    {
    case PackageType::STOCK_EXCHANGE_LIST:
        parseResult = parseStockExchangeList(answer, query.get());
        break;
    case  PackageType::KLINE_LIST:
        parseResult = parseKLineList(answer, query.get());
        break;
    case  PackageType::KLINE_NEW:
        parseResult = parseKLineNew(answer, query.get());
        break;
    case PackageType::KLINE_HISTORY:
        parseResult = parseKLineHistory(answer, query.get());
        break;
    case PackageType::SERVER_STATUS:
        parseResult = parseServerStatus(answer, query.get());
        break;
    case PackageType::UNDEFINED:
    default:
        Q_ASSERT(false);
    }

    if (parseResult.has_value())
    {
        emit errorOccurred(QString("The request to the server could not be completed. %1")
                               .arg(parseResult.value()), query->type(), id);
    }

    _package.erase(id);
}

void HTTPClient::errorOccurredHTTP(QNetworkReply::NetworkError code, quint64 serverCode, const QString &msg, quint64 id)
{
    Q_UNUSED(code);
    Q_UNUSED(serverCode);

    auto it_package = _package.find(id);
    Q_ASSERT(it_package != _package.end());

    emit sendLogMsg(TDBLoger::MSG_CODE::WARNING_CODE, QString("Error processing HTTP request: Package ID: %1 Message: %2").arg(id).arg(msg));

    auto& query = it_package->second;
    const auto packageType = query->type();
    switch (packageType)
    {
    case PackageType::STOCK_EXCHANGE_LIST:
    case PackageType::KLINE_LIST:
    case PackageType::KLINE_NEW:
    case PackageType::KLINE_HISTORY:
    case PackageType::SERVER_STATUS:
        if (!query->isEmptyAttemptions())
        {
            emit sendLogMsg(TDBLoger::MSG_CODE::WARNING_CODE, QString("The request to the server could not be completed. %1. Try: (%2/%3)")
                                                                  .arg(msg)
                                                                  .arg(query->totalTryCount() - query->tryCount())
                                                                  .arg(query->totalTryCount()));

            retryGetPackage(std::move(query));
        }
        else
        {
            emit errorOccurred(QString("The request to the server could not be completed. %1").arg(msg), query->type(), id);
        }
        break;
    case PackageType::NULL_DATA:
        break;
    case PackageType::UNDEFINED:
    default:
        Q_ASSERT(false);
    }

    _package.erase(id);
}

void HTTPClient::sendLogMsgHTTP(Common::TDBLoger::MSG_CODE category, const QString &msg, quint64 id)
{
    emit sendLogMsg(category, QString("HTTP client of Stock exchange. Request ID: %1 Message: %2").arg(id).arg(msg));
}

HTTPClient::~HTTPClient()
{
}

quint64 HTTPClient::sendServerStatus()
{
    auto query = std::make_unique<ServerStatusQuery>();

    return getPackage(std::move(query));
}

quint64 HTTPClient::sendStockExchangeList()
{
    auto query = std::make_unique<StockExchangesQuery>();

    return getPackage(std::move(query));
}

quint64 HTTPClient::sendKLineList(const StockExchangeID stockExchangeId)
{
    auto query = std::make_unique<KLinesListQuery>(stockExchangeId);

    return getPackage(std::move(query));
}

quint64 HTTPClient::sendKLineNew(const TradingCatCommon::KLineTypes& types /* = {TradingCatCommon::KLineType::MIN1} */, qint64 lastGetId /* = 0 */)
{
    auto query = std::make_unique<KLineNewQuery>(lastGetId == 0 ? _lastGetKLineId : lastGetId, types);

    return getPackage(std::move(query));
}

quint64 HTTPClient::sendKLineHistory(const StockExchangeID stockExchangeID, const KLineID klineId, const QDateTime &start, const QDateTime &end)
{
    auto query = std::make_unique<KLineHistoryQuery>(stockExchangeID, klineId, start, end);

    return getPackage(std::move(query));
}

bool HTTPClientConfig::isCheck() const noexcept
{
    return !address.isNull() && port != 0;
}
