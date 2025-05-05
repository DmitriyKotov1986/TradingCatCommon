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
    struct KLineDetectData
    {
        TradingCatCommon::StockExchangeID stockExchangeId;
        TradingCatCommon::Filter::FilterTypes filterActivate;
        TradingCatCommon::PKLinesList history;
        TradingCatCommon::PKLinesList reviewHistory;
        double delta = 0.0f;
        double volume = 0.0f;
        QString msg;
    };

    using PKLineDetectData = std::shared_ptr<KLineDetectData>;

    struct KLinesDetectedList
    {
        std::list<TradingCatCommon::Detector::PKLineDetectData> detected;
        bool isFull = false;

        void clear();
    };

public:
    /*!
        Конструтор
        @param parent - указатель на родительский класс
     */
    explicit Detector(const TradingCatCommon::TradingData& tradingData, QObject *parent = nullptr);

public slots:
    void start();
    void stop();

    void addKLines(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klines);

    void userOnline(qint64 sessionId, const TradingCatCommon::UserConfig& config);
    void userOffline(qint64 sessionId);

signals:
    /*!
        Сообщение логеру
        @param category - категория сообщения
        @param msg - текст сообщения
    */
    void sendLogMsg(Common::TDBLoger::MSG_CODE category, const QString& msg);

    /*!
        Сигнал генерируется если в процессе работы сервера произошла фатальная ошибка
        @param errorCode - код ошибки
        @param errorString - текстовое описание ошибки
    */
    void errorOccurred(Common::EXIT_CODE errorCode, const QString& errorString);

    void klineDetect(qint64 sessionId, const TradingCatCommon::Detector::PKLineDetectData& detectData);

    void finished();

private:
    const TradingCatCommon::TradingData& _tradingData;

    std::unordered_map<qint64, TradingCatCommon::Filter> _filters;

    std::unordered_map<TradingCatCommon::KLineID, QDateTime> _alreadyDetectKLine; ///< Свечи на которых детектор сработал за последнии 10 минут

}; //class Detector

} // namespace TradingCatCommon

Q_DECLARE_METATYPE(TradingCatCommon::Detector::PKLineDetectData);
Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
