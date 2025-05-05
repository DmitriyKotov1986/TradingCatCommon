//Qt
#include <QSslKey>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

//My
#include "TradingCatCommon/transmitdata.h"

#include "TradingCatCommon/httpserver.h"

using namespace TradingCatCommon;
using namespace Common;

HttpServer::HttpServer(const HTTPServerConfig &serverConfig, const TradingCatCommon::KLinesData& data, QObject *parent /* = nullptr */)
    : QObject(parent)
    , _serverConfig(serverConfig)
    , _data(data)
{
}

HttpServer::~HttpServer()
{
    stop();
}

void HttpServer::start()
{
    Q_ASSERT(!_isStarted);

    if (!makeServer())
    {
        _server.reset();

        return;
    }

    _startDateTime = QDateTime::currentDateTime();
    _isStarted = true;
}

void HttpServer::stop()
{
    if (!_isStarted)
    {
        return;
    }

    _server.release();

    _isStarted = false;
}

bool HttpServer::makeServer()
{
    _server = std::make_unique<QHttpServer>();

    _server->route(StockExchangesQuery().path(),
                   [this](const QHttpServerRequest &request)
                   {
                       emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, QString("GET Request StockExchangeList from %2:%3")
                                                                                 .arg(request.remoteAddress().toString())
                                                                                 .arg(request.remotePort()));
                       return stockExchangeList();
                   });

    _server->route(KLinesListQuery().path(),
                   [this](const QHttpServerRequest &request)
                   {
                       emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, QString("GET Request KLineList from %2:%3")
                                                                                 .arg(request.remoteAddress().toString())
                                                                                 .arg(request.remotePort()));
                       return klineList(request);
                   });

    _server->route(KLineNewQuery().path(),
                   [this](const QHttpServerRequest &request)
                   {
                       emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, QString("GET Request KLineNew from %2:%3")
                                                                                 .arg(request.remoteAddress().toString())
                                                                                 .arg(request.remotePort()));
                       return klineNew(request);
                   });

    _server->route(KLineHistoryQuery().path(),
                   [this](const QHttpServerRequest &request)
                   {
                       emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, QString("GET Request KLineHistory from %2:%3")
                                                                                 .arg(request.remoteAddress().toString())
                                                                                 .arg(request.remotePort()));
                       return klineHistory(request);
                   });

    _server->route(ServerStatusQuery().path(),
                   [this](const QHttpServerRequest &request)
                   {
                       emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, QString("GET Request ServerStatus from from %2:%3")
                                                                                 .arg(request.remoteAddress().toString())
                                                                                 .arg(request.remotePort()));
                       return serverStatus();
                   });

    _server->route(OrderBookNewQuery().path(),
                   [this](const QHttpServerRequest &request)
                   {
                       emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, QString("GET Request OrderBookNew from from %2:%3")
                                                                                 .arg(request.remoteAddress().toString())
                                                                                 .arg(request.remotePort()));
                       return orderBookNew(request);
                   });

    _server->afterRequest([](QHttpServerResponse &&resp)
                          {
                              resp.addHeader("Server", QCoreApplication::applicationName().toUtf8());
                              resp.addHeader("Cross-Origin-Embedder-Policy","require-corp");
                              resp.addHeader("Cross-Origin-Opener-Policy", "same-origin");
                              resp.addHeader("Content-Type", "application/json");
#ifdef QT_DEBUG
                              resp.addHeader("Access-Control-Allow-Origin", "*");
                              resp.addHeader("Access-Control-Allow-Headers", "*");
#endif
                              return std::move(resp);
                          });

    if (!_serverConfig.crtFileName.isEmpty() && !_serverConfig.keyFileName.isEmpty())
    {
        const auto sslCertificateChain = QSslCertificate::fromPath(_serverConfig.crtFileName);
        if (sslCertificateChain.empty())
        {
            emit errorOccurred(EXIT_CODE::HTTP_SERVER_NOT_LOAD_SSL_CERTIFICATE, QString("Couldn't retrieve SSL certificate from file: %1")
                                                                                    .arg(_serverConfig.crtFileName));

            return false;
        }

        QFile privateKeyFile(_serverConfig.keyFileName);
        if (!privateKeyFile.open(QIODevice::ReadOnly))
        {
            emit errorOccurred(EXIT_CODE::HTTP_SERVER_NOT_LOAD_SSL_CERTIFICATE, QString("Couldn't open file for reading: %1. Error: %2")
                                                                                    .arg(_serverConfig.keyFileName)
                                                                                    .arg(privateKeyFile.errorString()));
            return false;
        }

        _server->sslSetup(sslCertificateChain.front(), QSslKey(&privateKeyFile, QSsl::Rsa));

        privateKeyFile.close();
    }

    if (!_server->listen(_serverConfig.address, _serverConfig.port))
    {
        emit errorOccurred(EXIT_CODE::HTTP_SERVER_NOT_LISTEN, QString("Cannot listen application server on: %1:%2")
                                                                  .arg(_serverConfig.address.toString())
                                                                  .arg(_serverConfig.port));

        return false;
    }

    emit sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, QString("Server listened on %1:%2 successfully").arg(_serverConfig.address.toString()).arg(_serverConfig.port));

    return true;
}

QString HttpServer::serverStatus() const
{
    const auto currDateTime = QDateTime::currentDateTime();
    const auto appName = QString("%1 (Total money: %2)")
                             .arg(_serverConfig.name.isEmpty() ? QCoreApplication::applicationName() : _serverConfig.name)
                             .arg(_data.moneyCount());
    ServerStatusJson statusJson(appName, QCoreApplication::applicationVersion(), currDateTime, _startDateTime.secsTo(currDateTime));

    return Package(statusJson).toJson();
}

QString HttpServer::stockExchangeList() const
{
    return Package(_data.getStockExchangeList()).toJson();
}

QString HttpServer::klineList(const QHttpServerRequest &request) const
{
    const auto query = request.query();

    KLinesListQuery queryData(query);

    if (queryData.isError())
    {
        return Package(StatusJson::ErrorCode::BAD_REQUEST, queryData.errorString()).toJson();
    }

    return Package(_data.getKLineList(queryData.stockExchageId())).toJson();
}

QString HttpServer::klineNew(const QHttpServerRequest &request) const
{
    const auto query = request.query();

    KLineNewQuery queryData(query);

    if (queryData.isError())
    {
        return Package(StatusJson::ErrorCode::BAD_REQUEST, queryData.errorString()).toJson();
    }

    return Package(_data.getNewKLine(queryData.lastGetId(), queryData.maxCount(), queryData.types())).toJson();
}

QString HttpServer::klineHistory(const QHttpServerRequest &request) const
{
    const auto query = request.query();

    KLineHistoryQuery queryData(query);

    if (queryData.isError())
    {
        return Package(StatusJson::ErrorCode::BAD_REQUEST, queryData.errorString()).toJson();
    }

    return Package(_data.getKLinesOnDate(queryData.stockExchangeID(), queryData.klineID(), queryData.start(), queryData.end())).toJson();
}

QString HttpServer::orderBookNew(const QHttpServerRequest &request) const
{
    const auto query = request.query();

    OrderBookNewQuery queryData(query);

    if (queryData.isError())
    {
        return Package(StatusJson::ErrorCode::BAD_REQUEST, queryData.errorString()).toJson();
    }

    return Package(_data.getNewOrderBook(queryData.lastGetId(), queryData.maxCount(), queryData.types())).toJson();
}
