//QT
#include <QFileInfo>
#include <QSettings>
#include <QDir>

//My
#include "Common/common.h"

#include "httpservice.h"

using namespace HTTPServerService;
using namespace Common;

HTTPService::HTTPService(int& argc, char **argv)
    : QObject{nullptr}
    , QtService(argc, argv, "HTTPServer")
    , _cnf(Config::config())
{
    Q_CHECK_PTR(_cnf);

    setServiceDescription("HTTP Server service");
    setServiceFlags(QtServiceBase::CanBeSuspended);

    //настраиваем подключение БД логирования
    const auto& dbConnectionInfo = _cnf->dbConnectionInfo();
    auto logdb = QSqlDatabase::addDatabase(dbConnectionInfo.db_Driver, "LogDB");
    logdb.setDatabaseName(dbConnectionInfo.db_DBName);
    logdb.setUserName(dbConnectionInfo.db_UserName);
    logdb.setPassword(dbConnectionInfo.db_Password);
    logdb.setConnectOptions(dbConnectionInfo.db_ConnectOptions);
    logdb.setPort(dbConnectionInfo.db_Port);
    logdb.setHostName(dbConnectionInfo.db_Host);

    _loger = Common::TDBLoger::DBLoger(_cnf->dbConnectionInfo(), "HTTPServerLog", _cnf->sys_DebugMode());

    QObject::connect(_loger, SIGNAL(errorOccurred(Common::EXIT_CODE, const QString&)),
                     SLOT(errorOccurredLoger(Common::EXIT_CODE, const QString&)));

    _loger->start();

    if (_loger->isError())
    {
        _errorString = QString("Loger initialization error. Error: %1").arg(_loger->errorString());
    }
}

HTTPService::~HTTPService()
{
    if (_isRun)
    {
        this->stop();
    }

    if (_loger != nullptr)
    {
        _loger->sendLogMsg(TDBLoger::MSG_CODE::OK_CODE, "Successfully finished");

        TDBLoger::deleteDBLoger();
    }
}

QString HTTPService::errorString()
{
    const auto res = _errorString;
    _errorString.clear();

    return res;
}

void HTTPService::start()
{
    Q_CHECK_PTR(_cnf);
    Q_CHECK_PTR(_loger);

    if (_isRun)
    {
        _loger->sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, "Service already run. Start ignored");

        return;
    }

    _loger->sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, "Service start");

    try
    {
        _httpServer = new HTTPServer();

        QObject::connect(_httpServer, SIGNAL(errorOccurred(Common::EXIT_CODE, const QString&)),
                         SLOT(errorOccurredServer(Common::EXIT_CODE, const QString&)));

        _httpServer->start();

        _isRun = true;
    }
    catch (const std::exception &e)
    {
        _loger->sendLogMsg(TDBLoger::MSG_CODE::CRITICAL_CODE, QString("Critical error start service: %1").arg(e.what()));

        exit(EXIT_CODE::SERVICE_START_ERR);
    }
}

void HTTPService::pause()
{
    stop();
}

void HTTPService::resume()
{
    start();
}

void HTTPService::stop()
{
    Q_CHECK_PTR(_loger);

    if (_loger == nullptr)
    {
        return;
    }

    if (!_isRun)
    {
        _loger->sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, "Service is not runing. Stop ignored");

        return;
    }

    _loger->sendLogMsg(TDBLoger::MSG_CODE::INFORMATION_CODE, "Service stop");

    Q_CHECK_PTR(_httpServer);

    try
    {
        _httpServer->stop();

        delete _httpServer;

        _httpServer = nullptr;
    }
    catch (const std::exception &e)
    {
        _loger->sendLogMsg(TDBLoger::MSG_CODE::CRITICAL_CODE, QString("Critical error stop service: %1").arg(e.what()));

        exit(EXIT_CODE::SERVICE_STOP_ERR);
    }
}

void HTTPService::errorOccurredLoger(Common::EXIT_CODE errorCode, const QString &errorString)
{
    QString msg = QString("Critical error while the loger is running. Code: %1 Message: %2").arg(errorCode).arg(errorString);
    qCritical() << QString("%1 %2").arg(QTime::currentTime().toString(SIMPLY_TIME_FORMAT)).arg(msg);
    Common::writeLogFile("ERR>", msg);

    exit(errorCode);
}

void HTTPService::errorOccurredServer(Common::EXIT_CODE errorCode, const QString &errorString)
{
    QString msg = QString("Critical error while the server is running. Code: %1 Message: %2").arg(errorCode).arg(errorString);
    qCritical() << QString("%1 %2").arg(QTime::currentTime().toString(SIMPLY_TIME_FORMAT)).arg(msg);
    _loger->sendLogMsg(Common::TDBLoger::MSG_CODE::CRITICAL_CODE, msg);

    exit(errorCode);
}
