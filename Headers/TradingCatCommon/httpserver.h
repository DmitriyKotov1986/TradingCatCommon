#pragma once

//STL
#include <memory>
#include <unordered_map>
#include <queue>

//QT
#include <QtNetwork/QTcpServer>
#include <QCoreApplication>

//My
#include "TradingCatCommon/tradingdata.h"
#include "TradingCatCommon/socketthread.h"
#include "TradingCatCommon/thread.h"
#include "TradingCatCommon/types.h"

namespace TradingCatCommon
{

class HttpServer
    : public QTcpServer
{
    Q_OBJECT

public:
    /*!
        Конструктор. Планируется использовать только это тконструтор
        @param serverConfig - конфигурация сервера
        @param data - данные
        @param parent - указатель на родительский класс
     */
    HttpServer(const TradingCatCommon::HTTPServerConfig &serverConfig, const TradingCatCommon::TradingData& data, QObject *parent = nullptr);

    /*!
        Деструктор
    */
    ~HttpServer();

public slots:
    /*!
        Старт сервера (листинг и создание потоков соединения).
    */
    void start();

    /*!
        Остановка сервера и закрытие соединений
     */
    void stop();

signals:
    /*!
        Сообщение логеру
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsg(Common::MSG_CODE category, const QString& msg);

    /*!
        Сигнал генерируется если в процессе работы сервера произошла фатальная ошибка
        @param errorCode - код ошибки
        @param errorString - текстовое описание ошибки
    */
    void errorOccurred(Common::EXIT_CODE errorCode, const QString& errorString);

    /*!
        Остановка сервера
    */
    void stopAll();

    /*!
        Сервер успешно остановлени все соединения закрыты
    */
    void finished();

    /*!
        Пришло новое входящее подключение. Сигнал отправляеться потоку обрабатывающему соединение
        @param id - ИД потока
        @param handle - ИД соединения
     */
    void startSocket(quint64 id, qintptr handle);

private slots:
    /*!
        Вызываетяс когда поток соединения успешно запустился и готов принимать соединения
        @param id - ИД потока
    */
    void startedConnecton(quint64 id);

    /*!
        Вызывается когда поток закончил обрботку соединения и готов принимать новое
        @param id - ИД потока
    */
    void finishedSocket(quint64 id);

    /*!
        Сообщение логеру от обработчика входящих соединений
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsgSocket(Common::MSG_CODE category, const QString &msg);

private:
    // Удаляем неиспользуемые конструкторы
    HttpServer() = delete;
    Q_DISABLE_COPY_MOVE(HttpServer);

    /*!
        Нативные обработчик входящего соединения
        @param handle - уникальный идентификатор соединения
    */
    void incomingConnection(qintptr handle) override;

private:
    /*!
        Обработчик входящего соединения
    */
    struct Connection
    {
        std::unique_ptr<TradingCatCommon::Thread> thread;       ///< Поток обработки соединения
        std::unique_ptr<TradingCatCommon::SocketThread> socket; ///< класс обработки данных соединения
    };

    using PConnection = std::unique_ptr<Connection>;            ///< указатель на соединение

private:
    std::unordered_map<quint64, PConnection> _poolConnections;  ///< Пулл потоков соединий

    std::queue<Connection*> _freeConnections;                   ///< очередь свободных соединений
    std::unordered_map<quint64, Connection*> _busyConnections;  ///< очередь соединеий занятых обработкой данных

    const HTTPServerConfig& _serverConfig;                      ///< Конфигуация сервера
    const TradingCatCommon::TradingData& _data;                 ///< ССылка на объект данных

    QDateTime _startDateTime = QDateTime::currentDateTime();    ///< Время запуска сервера

    bool _isStarted = false;                                    ///< флаг усешного запуска

};

}
