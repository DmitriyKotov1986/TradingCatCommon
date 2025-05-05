#pragma once

//STL
#include <memory>

//QT
#include <QObject>
#include <QSqlDatabase>
#include <QTimer>

//My
#include <Common/common.h>
#include <Common/sql.h>
#include <Common/tdbloger.h>

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     The DBCleaner class - класс выполняет периодискую очистку БД со свечами
///
class DBCleaner final
    : public QObject
{
    Q_OBJECT

public:
    /*!
        Конструктор. Планируется использовать только этот конструтор
        @param dbConnectionInfo - параметры подключения к БД. Значение должно быть корректным
        @param parent - указатель на родительски класс
    */
    explicit DBCleaner(const Common::DBConnectionInfo& dbConnectionInfo, QObject *parent = nullptr);

    /*!
        Деструктор
    */
    ~DBCleaner();

    /*!
        Запускае очистку
     */
    void start();

    /*!
        Останавливает очистку
    */
    void stop();

signals:
    /*!
        Сигнал генерируется если в процессе работы с БД произошла ошибка
        @param errorCode - код ошибки
        @param errorString - текстовое описание ошибки
    */
    void errorOccurred(Common::EXIT_CODE errorCode, const QString& errorString);

    /*!
        Дополнительное сообщение логеру
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsg(Common::TDBLoger::MSG_CODE category, const QString& msg);

private slots:
    /*!
        Удаляет старые запии из БД
    */
    void clear();

private:
    // Удаляем неиспользуемые конструкторы
    DBCleaner() = delete;
    Q_DISABLE_COPY_MOVE(DBCleaner);

private:
    const Common::DBConnectionInfo _dbConnectionInfo; ///< Параметры подлючения к БД
    QSqlDatabase _db;                                 ///< подключеие к БД

    std::unique_ptr<QTimer> _timer; ///< таймер очистки

    bool _isStarted = false;        ///< Флаг успешного запуска

};

} //namespace TraidingCatBot
