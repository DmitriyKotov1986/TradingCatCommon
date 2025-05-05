#include "TradingCatCommon/httpserver.h"

using namespace TradingCatCommon;

static const quint32 MAX_CONNETIONS = 100;

HttpServer::HttpServer(const HTTPServerConfig &serverConfig, const TradingCatCommon::TradingData& data, QObject *parent /* = nullptr */)
    : QTcpServer(parent)
    , _serverConfig(serverConfig)
    , _data(data)
{
}

HttpServer::~HttpServer()
{
    stop();
}

void HttpServer::incomingConnection(qintptr handle)
{
    if (_freeConnections.size() == 0)
    {
         emit sendLogMsg(Common::TDBLoger::MSG_CODE::WARNING_CODE, QString("Too many connetions: %1").arg(MAX_CONNETIONS));

         auto tcpSocket = new QTcpSocket();
         tcpSocket->setSocketDescriptor(handle);

         QByteArray answer = "HTTP/1.1 429 Too Many Requests\r\nContent-Length:17\r\n\r\nToo Many Requests";

         tcpSocket->write(answer, answer.size());
         tcpSocket->waitForBytesWritten();
         tcpSocket->disconnectFromHost();

         return;
    }

    const auto p_connection = _freeConnections.front();

    _freeConnections.pop();

    emit startSocket(p_connection->socket->id(), handle);
}

void HttpServer::HttpServer::start()
{
    // for (quint32 i = 0; i < MAX_CONNETIONS; ++i)
    // {
    //     Connection tmp;

    //     tmp.thread = std::make_unique<Thread>(i); //создаем поток
    //     tmp.socket = std::make_unique<SocketThread>(i, _serverConfig, _data); //создаем класс обработчика соединения

    //     tmp.socket->moveToThread(tmp.thread.get()); //перемещаем обработчик в отдельный поток

    //     //запускаем обработку стазу после создания потока
    //     QObject::connect(tmp.thread.get(), SIGNAL(started(quint64)), SLOT(startedSocket(quint64)), Qt::QueuedConnection);


    //     //log
    //     QObject::connect(tmp.socket.get(), SIGNAL(sendLogMsg(Common::TDBLoger::MSG_CODE, const QString&)),
    //                  SLOT(sendLogMsgSocket(Common::TDBLoger::MSG_CODE, const QString&)), Qt::QueuedConnection);

    //     //Ставим поток на удаление когда завершили работы
    //     QObject::connect(tmp.socket.get(), SIGNAL(finished()), tmp.thread.get(), SLOT(quit()), Qt::DirectConnection);

    //     //тормозим поток при отключении сервера
    //     QObject::connect(this, SIGNAL(stopAll()), tmp.socket.get(), SLOT(stop()), Qt::QueuedConnection);



    //     //запускаем поток на выполнение
    //     const auto p_thread = tmp.thread.get();

    //     _poolConnections.emplace(i, std::move(tmp));

    //     p_thread->start(QThread::NormalPriority);
    // }

    // if (listen(_serverConfig.address, _serverConfig.port))
    // {
    //     emit sendLogMsg(Common::TDBLoger::MSG_CODE::INFORMATION_CODE, QString("The server was listening on %1:%2").arg(_serverConfig.address.toString()).arg(_serverConfig.port));
    // }
    // else
    // {
    //     emit errorOccurred(Common::EXIT_CODE::SERVICE_START_ERR, QString("Error listening server on %1:%2").arg(_serverConfig.address.toString()).arg(_serverConfig.port));
    // }

    _isStarted = true;
}

void HttpServer::stop()
{
    if (!_isStarted)
    {
        return;
    }

    close(); //Закрываем все соединения

    emit stopAll(); //тормозим все потоки

    for (const auto& connection: _poolConnections)
    {
        connection.second->thread->wait();
    }

    _poolConnections.clear();

    emit sendLogMsg(Common::TDBLoger::MSG_CODE::INFORMATION_CODE,  "Server is stoped succesfull");

    emit finished();
}

void HttpServer::startedConnecton(quint64 id)
{

}

// void HttpServer::startedSocket(quint64 id)
// {
//     const auto connections_it = _poolConnections.find(id);
//     Q_ASSERT(connections_it != _poolConnections.end());

//     _freeConnections.push(connections_it->second.get());
// }

void HttpServer::sendLogMsgSocket(Common::TDBLoger::MSG_CODE category, const QString &msg)
{
    emit sendLogMsg(category, msg);
}

void HttpServer::finishedSocket(quint64 id)
{
    const auto connections_it = _busyConnections.find(id);
    Q_ASSERT(connections_it != _busyConnections.end());

    _freeConnections.push(connections_it->second);
}

