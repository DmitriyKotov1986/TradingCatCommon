#pragma once

//STL
#include <unordered_map>
#include <memory>

//Qt
#include <QObject>

//My
#include <Common/httpsslquery.h>

#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/ikline.h"
#include "TradingCatCommon/tradingdata.h"

#if !QT_NO_SSL

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
///     KLinePool class - пул свечей для получения данных от биржи
///
class KLineHTTPPool
    : public QObject
{
    Q_OBJECT

public:
    explicit KLineHTTPPool(Common::HTTPSSLQuery* http, QObject* parent = nullptr);
    ~KLineHTTPPool();

    /*!
        Устанавливает имя пользователя и пароль для автоизации на сервере, если она требется. Этот метод должен быть вызван до выполнения первого запроса.
        @param user - имя пользователя
        @param password - пароль
    */
    void setUserPassword(const QString& user, const QString& password);

    /*!
        Добавляет свечь в пул
        @param kline - указател на клас получения данных о свече с биржи
    */
    void addKLine(std::unique_ptr<TradingCatCommon::IKLine>&& kline);

    /*!
        Удаляет свечй из пула
        @param id - ИД свечи
    */
    void deleteKLine(const TradingCatCommon::KLineID& id);

    /*!
        Возвращает true - если свеча есть в пуле
        @param id - ИД свечи
        @return true - если свеча есть в пул, false - нет
    */
    bool isExitsKLine(const TradingCatCommon::KLineID& id) const;

    /*!
        Возвращает список свечей добавленных в пул
        @return список свечей ИД добавленных в пул
    */
    TradingCatCommon::KLinesIDList addedKLines() const;

    /*!
        Возвращает текущее количество обрабатываемых пулом свечей
        @return количество свечей в пуле
     */

    qsizetype klineCount() const noexcept;

signals:
    /*!
        Получены новые свечи от биржи. Гарантируются что свечи не повторяются и не равны ранее полученным,
            а также что они отсортированы по возрастанию времени
        @param klines - список свечей
    */
    void getKLines(const TradingCatCommon::PKLinesList& klines);

    /*!
        Произошла фатальная ошибка при получении данных от биржи
        @param errorCode
        @param errorString
    */
    void errorOccurred(Common::EXIT_CODE errorCode, const QString& errorString);

    /*!
        Необходимости записи сообщения в лог
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsg(Common::MSG_CODE category, const QString& msg);

private slots:
    // from IKLine
    /*!
        Сигнал получения новых свечей
        @param klines - список свечей    else if (stockExchangeName == "BITMART")
    {
        return QColor(47, 93, 200); //синий
    }
    */
    void getKLinesKLine(const TradingCatCommon::PKLinesList& klines);

    void errorOccurredKLine(const TradingCatCommon::KLineID& id, Common::EXIT_CODE errorCode, const QString& errorString);
    void sendLogMsgKLine(const TradingCatCommon::KLineID& id, Common::MSG_CODE category, const QString& msg);



private:
    // Удаляем неиспользуемые конструкторы
    Q_DISABLE_COPY_MOVE(KLineHTTPPool);

private:
    std::unordered_map<TradingCatCommon::KLineID, std::unique_ptr<IKLine>> _klines; ///< Карта свечей в пуле. Ключ - ИД свиди, Значение - указател указатель на класс получения данных с биржи

    Common::HTTPSSLQuery* _http = nullptr; ///< Указатель на класс обработки HTTP запросов

};

} // namespace TradingCatCommon

#endif
