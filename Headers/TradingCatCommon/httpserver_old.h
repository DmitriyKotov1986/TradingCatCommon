#pragma once

//STL
#include <memory>

//Qt
#include <QCoreApplication>
#include <QObject>
#include <QtHttpServer/QHttpServer>

//My
#include <Common/common.h>
#include <Common/tdbloger.h>

#include "TradingCatCommon/klinesdata.h"

namespace TradingCatCommon
{

struct HTTPServerConfig
{
    QHostAddress address = QHostAddress::LocalHost;
    quint16 port = 80;
    quint64 maxUsers = 1000;
    QString rootDir = QCoreApplication::applicationDirPath();
    QString crtFileName;
    QString keyFileName;
    QString name;
};

class HttpServer
    : public QObject
{
    Q_OBJECT

public:
    explicit HttpServer(const HTTPServerConfig& serverConfig, const TradingCatCommon::KLinesData& data, QObject* parent = nullptr);

    /*!
        Деструктор
    */
    ~HttpServer();

    void start();
    void stop();    

signals:
    void errorOccurred(Common::EXIT_CODE errorCode, const QString& errorString);
    void sendLogMsg(Common::TDBLoger::MSG_CODE category, const QString& msg);

private:
    HttpServer() = delete;
    Q_DISABLE_COPY_MOVE(HttpServer);

    bool makeServer();

    QString serverStatus() const;
    QString stockExchangeList() const;
    QString klineList(const QHttpServerRequest &request) const;
    QString klineNew(const QHttpServerRequest &request) const;
    QString klineHistory(const QHttpServerRequest &request) const;
    QString orderBookNew(const QHttpServerRequest &request) const;

private:
    const HTTPServerConfig& _serverConfig;
    const TradingCatCommon::KLinesData& _data;

    std::unique_ptr<QHttpServer> _server;

    QDateTime _startDateTime = QDateTime::currentDateTime();

    bool _isStarted = false;
};


} // namespace TradingCatCommon
