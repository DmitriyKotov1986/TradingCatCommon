#pragma once

//Qt
#include <QObject>

//My
#include <Common/httpsslquery.h>

#include "TradingCatCommon/kline.h"

#ifndef QT_NO_SSL

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     IKLine class - класс интерфейса обработки запросов на получение
///     сведений с бирже по свечам. Потомок данного класса должен обеспечивать получение сведений
///     о новых свечах с биржи. При получении новых сведений отправлять их помощью
///     генерации сигнала getKLines(...). Сведения о свечах не должны повторятся и должны быть
///     отсортироавнны в порядке возрастания времени. Обмен с биржей должен происходить с помощьюъ
///     экземпляра класса HTTPSSLQuery, указатель на который можно полуить с помощью
///     вызова getHTTP().
///
class IKLine
    : public QObject
{
    Q_OBJECT

public:
    /*!
        Конструктор. Предполагается использование только этого конструктора
        @param id - ИД Свечи. Не должно бфть пусным
        @param parent - указатель на родительский класс
    */
    IKLine(const TradingCatCommon::KLineID& id, QObject* parent = nullptr);

    /*!
        Возвращает ИД свечи. Гарантируется что ИД не булет пустым
        @return ИД свечи
    */
    const TradingCatCommon::KLineID& id() const;

    /*!
        Устанавливает указатель на класс обработки HTTP запросов. Указатель не должен быть nullptr
        @param http - указатель на класс обработки запросов
    */
    void setHTTP(Common::HTTPSSLQuery* http);

    /*!
        Возвращает указатель на класс обработки HTTP запросов.
        @return - указатель на класс обработки HTTP запросов.
     */
    Common::HTTPSSLQuery* getHTTP() const noexcept;

    /*!
        Этот метод будет вызван перед началом обмена. В момент вызова этого метода
            гарантируется что getHTTP() не вернет nullptr
     */
    virtual void start();

    /*!
        Этот метод будет вызван перед завершением обмена
     */
    virtual void stop();

protected:
    /*!
        Метод вызываемый наследником при полуении новых данных о свечах от биржи.
        @param klines - список свечей. Не должен быть пустым и свечи должны быть упорядочены по возрастанию даты
    */
    void addKLines(const TradingCatCommon::PKLinesList& klines);

signals:
    /*!
        Данный сигнал следут генерировать в случае фатальной ошибки обмены с биржей
        @param id - ИД свеси
        @param errorCode - код ошибки
        @param errorString - сообщение об ошибке
    */
    void errorOccurred(const TradingCatCommon::KLineID& id, Common::EXIT_CODE errorCode, const QString& errorString);

    /*!
        Данный сигнал следут генерировать в случаее необходимости записи сообщения в ло
        @param id - ИД свеси
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsg(const TradingCatCommon::KLineID& id, Common::TDBLoger::MSG_CODE category, const QString& msg);

    /*!
        Сигнал получения новых свечей
        @param klines - список свечей
    */
    void getKLines(const TradingCatCommon::PKLinesList& klines);

private:
    // Удаляем неиспользуемые конструкторы
    IKLine() = delete;
    Q_DISABLE_COPY_MOVE(IKLine);

private:
    const TradingCatCommon::KLineID _id; ///< ИД Свечи

    Common::HTTPSSLQuery* _http = nullptr; ///< Указатель на класс обработки http запросов

};

} // namespace TradingCatCommon

#endif
