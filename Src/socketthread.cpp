//QT
#include <QFile>
#include <QProcess>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QByteArray>

//My
#include "TradingCatCommon/httpanswer.h"
#include "TradingCatCommon/transmitdata.h"

#include "TradingCatCommon/socketthread.h"

using namespace TradingCatCommon;
using namespace Common;

static const quint64 TIMEOUT_RECEIVE_DATA = 30 * 1000;
static const quint64 TIMEOUT_TRANSMIT_DATA = 30 * 1000;

SocketThread::SocketThread(quint64 id, const HTTPServerConfig &serverConfig, const TradingCatCommon::TradingData& data, QObject *parent /* = nullptr */)
    : QObject(parent)
    , _id(id)
    , _serverConfig(serverConfig)
    , _data(data)
{
}

SocketThread::~SocketThread()
{
    stop();
}

quint64 SocketThread::id() const noexcept
{
    return _id;
}

void SocketThread::stop()
{

}

void SocketThread::startSocket(qintptr handle)
{
    _isFirstPacket = true;

    //Создаем сокет
    _tcpSocket = new QTcpSocket();
    _tcpSocket->setSocketDescriptor(handle);

    QObject::connect(_tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));  //пришли новые данные
    QObject::connect(_tcpSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
                     this, SLOT(errorOccurred(QAbstractSocket::SocketError)));  //ошибка соединения

    //Создаем ватчдог
    _watchDog = new QTimer();
    _watchDog->setSingleShot(true);

    QObject::connect(_watchDog, SIGNAL(timeout()), this, SLOT(timeout()));  //таймаут соединения

    _request = new HTTPRequest(); //Класс приемник данных;

    emit sendLogMsg(Common::TDBLoger::MSG_CODE::INFORMATION_CODE,
                    QString("%1 Incomming connect. Client: %2:%3")
                        .arg(handle)
                        .arg(_tcpSocket->peerAddress().toString())
                        .arg(_tcpSocket->peerPort()));

   _watchDog->start(TIMEOUT_RECEIVE_DATA); //запускаем WatchDog
}

void SocketThread::parseResource()
{
    const auto& resource = _request->resurce();
    const auto path = resource.path();
    const auto query = QUrlQuery(resource.query());
    if (path == ServerStatusQuery().path())
    {
        sendAnswer(200, serverStatus().toUtf8());
    }
    else if (path == StockExchangesQuery().path())
    {
        sendAnswer(200, stockExchangeList().toUtf8());
    }
    else if (path == KLinesListQuery().path())
    {
        sendAnswer(200, klineList(query).toUtf8());
    }
    else if (path == KLineNewQuery().path())
    {
        sendAnswer(200, klineNew(query).toUtf8());
    }
    else if (path == KLineHistoryQuery().path())
    {
        sendAnswer(200, klineHistory(query).toUtf8());
    }
    else
    {
        sendAnswer(404, QString("%1 not found\n\r").arg(path).toUtf8());
    }
}

void SocketThread::sendAnswer(quint16 code, const QByteArray& msg)
{
    Q_CHECK_PTR(_tcpSocket);

    HTTPAnswer answer(code);
    answer.addBody(msg);

    const auto  sendBytes = _tcpSocket->write(answer.getAnswer());
    if (!_tcpSocket->waitForBytesWritten(TIMEOUT_TRANSMIT_DATA))  //ждем записи данных в буфер
    {
        const auto msg = QString("Data transmitting timeout. Transmit: %1 bytes").arg(sendBytes);

        emit sendLogMsg(TDBLoger::MSG_CODE::WARNING_CODE, QString("%1 %2").arg(_handle).arg(msg));
    }

/*    emit sendLogMsg(code == 200 ? TDBLoger::MSG_CODE::INFORMATION_CODE : TDBLoger::MSG_CODE::WARNING_CODE,
                    QString("%1 Finished. Reseived: %2 B. Transmited: %3 B. Return code: %9%10")
                        .arg(_socketDiscriptor)
                        .arg(_request->size())
                        .arg(sendBytes)
                        .arg(code)
                        .arg(code == 200 ? "" : QString(". Message: %1").arg(msg)));
*/

    finishSocket();
}

void SocketThread::finishSocket()
{
    //вызываем дисконнект
    if (_tcpSocket->isOpen())
    {
        _tcpSocket->disconnectFromHost();
    }

    delete _tcpSocket;
    _tcpSocket = nullptr;

    delete _request;
    _request = nullptr;

    delete _watchDog;
    _watchDog = nullptr;

    //Сообщаем о завершении
    emit finishedSocket(_id);
}

void SocketThread::readyRead()
{
     //перезапускаем WatchDog
    _watchDog->stop();

    //считываем пришедшие данные
    _request->add(_tcpSocket->readAll());
    if (_isFirstPacket)
    {
        _isFirstPacket = false;

        if (!_request->isGetHeader())
        {

            sendAnswer(_request->errorCode(), QString("Incorrect request. %1").arg(_request->errorMsg()).toUtf8());

            return;
        }
    }

    //если пришли все данные
    if (_request->isComplite())
    {
        if (_request->errorCode() != 200)
        {
            sendAnswer(_request->errorCode(), _request->errorMsg().toUtf8());

            return;
        }

        parseResource();
    }
}

void SocketThread::errorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    emit sendLogMsg(TDBLoger::MSG_CODE::WARNING_CODE,
                    QString("%1 Socket error: %2. Expected to receive: %3 B. Reseived: %4")
                        .arg(_handle)
                        .arg(_tcpSocket->errorString())
                        .arg(_request->expectedSize())
                        .arg(_request->size()));

    stop();
}


void SocketThread::timeout()
{
    const auto msg = QString("Data receiving timeout");

    emit sendLogMsg(TDBLoger::MSG_CODE::WARNING_CODE, QString("%1 %2").arg(_handle).arg(msg));

    sendAnswer(524, msg.toUtf8()); //timeout
}

QString SocketThread::serverStatus() const
{
    const auto currDateTime = QDateTime::currentDateTime();
    const auto appName = QString("%1 (Total money: %2)")
                             .arg(_serverConfig.name.isEmpty() ? QCoreApplication::applicationName() : _serverConfig.name)
                             .arg(_data.moneyCount());
    ServerStatusJson statusJson(appName, QCoreApplication::applicationVersion(), currDateTime, _serverConfig.startDateTime.secsTo(currDateTime));

    return Package(statusJson).toJson();
}

QString SocketThread::stockExchangeList() const
{
    return Package(_data.getStockExchangeList()).toJson();
}

QString SocketThread::klineList(const QUrlQuery& query) const
{
    KLinesListQuery queryData(query);

    if (queryData.isError())
    {
        return Package(StatusJson::ErrorCode::BAD_REQUEST, queryData.errorString()).toJson();
    }

    return Package(_data.getKLineList(queryData.stockExchageId())).toJson();
}

QString SocketThread::klineNew(const QUrlQuery& query) const
{
    // KLineNewQuery queryData(query);

    // if (queryData.isError())
    // {
    //     return Package(StatusJson::ErrorCode::BAD_REQUEST, queryData.errorString()).toJson();
    // }

    // return Package(_data.getNewKLine(queryData.lastGetId(), queryData.maxCount(), queryData.types())).toJson();
}

QString SocketThread::klineHistory(const QUrlQuery& query) const
{
    KLineHistoryQuery queryData(query);

    if (queryData.isError())
    {
        return Package(StatusJson::ErrorCode::BAD_REQUEST, queryData.errorString()).toJson();
    }

    return Package(_data.getKLinesOnDate(queryData.stockExchangeID(), queryData.klineID(), queryData.start(), queryData.end())).toJson();
}





