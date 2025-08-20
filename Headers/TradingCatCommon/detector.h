#pragma once

//STL
#include <unordered_map>

//Qt
#include <QObject>

//My
#include <Common/tdbloger.h>

#include "TradingCatCommon/stockexchange.h"
#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/filter.h"
#include "TradingCatCommon/tradingdata.h"
#include "TradingCatCommon/userconfig.h"

namespace TradingCatCommon
{

static const quint64 KLINES_COUNT_HISTORY = 300u;

///////////////////////////////////////////////////////////////////////////////
///     The Detector class - класс обеспечивает филтьтрацию свечей полученных с
///          бирж, расчет НАТРОВ и прочих велечин необходимых для выпонения фильрации
///          и генерацию  сигнала getNewKLine если свеча соотвествут нужным параметрам
///
class Detector
    : public QObject
{
    Q_OBJECT

public:
    /*!
        Структура данных сработовшего фильтра
    */
    struct KLineDetectData
    {
        TradingCatCommon::StockExchangeID stockExchangeId;      ///< ИД биржи
        TradingCatCommon::Filter::FilterTypes filterActivate;   ///< типы стработавших фильтров
        TradingCatCommon::PKLinesList history;                  ///< Список основных свечей
        TradingCatCommon::PKLinesList reviewHistory;            ///< Список свечей обзора
        float delta = 0.0f;    ///< Делта в момент стработки фильтра
        float volume = 0.0f;   ///< Объем в момент сработки фильтра
        QString msg;            ///< Отладочное сообщенеи
    };

    using PKLineDetectData = std::shared_ptr<KLineDetectData>; ///< список данных

    struct KLinesDetectedList
    {
        std::list<TradingCatCommon::Detector::PKLineDetectData> detected; ///< список данных
        bool isFull = false; ///< true  - если размер списка сработок достиг максимальной величины

        void clear(); ///< очищает структуру
    };

public:
    /*!
        Конструтор
        @param parent - указатель на родительский класс
     */
    explicit Detector(const TradingCatCommon::TradingData& tradingData, QObject *parent = nullptr);

    /*!
        Деструктор
    */
    ~Detector() override = default;

public slots:
    /*!
        Начало работы класса.
    */
    void start();

    /*!
        Завершение работы класса
    */
    void stop();

    /*!
        Слот обрабатывает новые полученные свечи
        @param stockExchangeId - ИД биржи. Должно гарантироваться что ИД биржи не пустое и валидно
        @param klines - список свечей. Должно гарантироваться что список не пустой
    */
    void addKLines(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klines);

    /*!
        Слот обработки начала сессии пользователя
        @param sessionId - ИД сессии пользователя. Не должно быть равно 0
        @param config - конфигурация пользователя
    */
    void userOnline(qint64 sessionId, const TradingCatCommon::UserConfig& config);

    /*!
        Завершенеи сессии пользователя
        @param sessionId - ИД сессии пользователя. Не должно быть равно 0
    */
    void userOffline(qint64 sessionId);

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
        Сигнал генерируется при обнаружении свечи соотвествующей фильтру.
        @param sessionId - ИД сессии пользователя
        @param detectData - данные детектирования свечи
    */
    void klineDetect(qint64 sessionId, const TradingCatCommon::Detector::PKLineDetectData& detectData);

    /*!
        Сингал генерируется после остановки работы класса
    */
    void finished();

private:
    const TradingCatCommon::TradingData& _tradingData; ///< ссылка на данные полученных свечей

    std::unordered_map<qint64, TradingCatCommon::Filter> _filters; ///< Список активных фильтрок. Ключ - ИД сессии пользователя, Значение - фильтр

}; //class Detector

} // namespace TradingCatCommon

Q_DECLARE_METATYPE(TradingCatCommon::Detector::PKLineDetectData);
Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
