#pragma once

//Ot
#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QHash>
#include <QTimer>
#include <QDateTime>
#include <QUrlQuery>

//My
#include "Common/tdbloger.h"
#include "TradingCatCommon/httprequest.h"
#include "TradingCatCommon/types.h"
#include "TradingCatCommon/tradingdata.h"

namespace TradingCatCommon
{

class SocketThread
    : public QObject
{
    Q_OBJECT

public:
    SocketThread(quint64 id, const HTTPServerConfig &serverConfig, const TradingCatCommon::TradingData& data, QObject *parent = nullptr);
    ~SocketThread();

    quint64 id() const noexcept;

public slots:
    void stop();
    void startSocket(qintptr handle);

private:
    void parseResource();

    void sendAnswer(quint16 code, const QByteArray& msg); //отправляет ответ

private slots:
    void readyRead();
    void errorOccurred(QAbstractSocket::SocketError socketError);

    void timeout();

signals:
    void sendLogMsg(Common::MSG_CODE category, const QString& msg);
    void finishedSocket(quint64 id);

private:
    void finishSocket();

    QString serverStatus() const;
    QString stockExchangeList() const;
    QString klineList(const QUrlQuery& query) const;
    QString klineNew(const QUrlQuery& query) const;
    QString klineHistory(const QUrlQuery& query) const;

private:
    const quint64 _id = 0;

    QTcpSocket* _tcpSocket = nullptr;
    TradingCatCommon::HTTPRequest* _request = nullptr;

    const TradingCatCommon::HTTPServerConfig& _serverConfig;
    const TradingCatCommon::TradingData& _data;

    bool _isFirstPacket = true;

    qintptr _handle = 0;

    QTimer* _watchDog = nullptr;

}; //class SocketThread

} //namespace TradingCatCommon
