#pragma once

//STL
#include <unordered_map>
#include <optional>

//Qt
#include <QObject>
#include <QHostAddress>
#include <QTimer>
#include <QDateTime>

//My
#include <Common/common.h>
#include <Common/tdbloger.h>
#include <Common/httpsslquery.h>

#include <TradingCatCommon/kline.h>
#include <TradingCatCommon/transmitdata.h>

namespace TradingCatCommon
{

struct HTTPClientConfig
{
    QHostAddress address = QHostAddress::LocalHost;
    quint16 port = 80;

    bool isCheck() const noexcept;
};

using HTTPClientConfigsList = std::list<HTTPClientConfig>;

class HTTPClient
    : public QObject
{
    Q_OBJECT

public:
    explicit HTTPClient(const HTTPClientConfig& config, QObject *parent = nullptr);

    /*!
        Деструктор
    */
    ~HTTPClient();

    quint64 sendServerStatus();
    quint64 sendStockExchangeList();
    quint64 sendKLineList(const TradingCatCommon::StockExchangeID stockExchangeId);
    quint64 sendKLineNew(const TradingCatCommon::KLineTypes& types = {TradingCatCommon::KLineType::MIN1}, qint64 lastGetId = 0);
    quint64 sendKLineHistory(const TradingCatCommon::StockExchangeID stockExchangeId,
                             const TradingCatCommon::KLineID klineId,
                             const QDateTime& start,
                             const QDateTime& end);

signals:
    void sendLogMsg(Common::TDBLoger::MSG_CODE category, const QString& msg);

    void stockExchangeList(const TradingCatCommon::StockExchangesIDList& stockExchangeIdList, quint64 id);
    void klineList(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::KLinesIDList& klineIdList, quint64 id);
    void klineNew(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klinesList, quint64 id);

    /*!
        Генерируется когда полученны данные с историей Свечей. Гарантируется что klinesList содержит свечи с одинаковым ИД и список не пустой
        @param stockExchangeId - ИД биржи
        @param klinesList - список свечей
        @param id - ИД запроса вернутый sendKLineHistory(...)
    */
    void klineHistory(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klinesList, quint64 id);
    void serverStatus(const QString& serverStatus, const QString& serverVersion, const QDateTime& serverTime, qint64 upTime, quint64 id);

    void errorOccurred(const QString& msg, TradingCatCommon::PackageType type, quint64 id);

private:
    HTTPClient() = delete;
    Q_DISABLE_COPY_MOVE(HTTPClient);

    quint64 getPackage(std::unique_ptr<Query>&& query);
    void retryGetPackage(std::unique_ptr<Query>&& query);

    using ParseResult = std::optional<QString>;

    ParseResult parseStockExchangeList(const QByteArray& answer, const Query* query);
    ParseResult parseKLineList(const QByteArray& answer, const Query* query);
    ParseResult parseKLineNew(const QByteArray& answer, const Query* query);
    ParseResult parseKLineHistory(const QByteArray& answer, const Query* query);
    ParseResult parseServerStatus(const QByteArray& answer, const Query* query);

private slots:
    void getAnswerHTTP(const QByteArray& answer, quint64 id);
    void errorOccurredHTTP(QNetworkReply::NetworkError code, quint64 serverCode, const QString& msg, quint64 id);
    void sendLogMsgHTTP(Common::TDBLoger::MSG_CODE category, const QString& msg, quint64 id);

private:
    const HTTPClientConfig _config;

    Common::HTTPSSLQuery* _http = nullptr;

    std::unordered_map<quint64, std::unique_ptr<TradingCatCommon::Query>> _package;

    qint64 _lastGetKLineId = 0;
    qint64 _lastGetOrderBookId = 0;


    StockExchangesIDList _stockExchangeIdList;

}; // class HTTPClient

} // namespace TradingCatCommon
