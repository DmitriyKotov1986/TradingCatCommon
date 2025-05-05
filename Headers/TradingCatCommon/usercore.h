//Qt
#include <QObject>
#include <QJsonArray>
#include <QDateTime>

//My
#include "TradingCatCommon/detector.h"
#include "TradingCatCommon/httpclient.h"

namespace TradingCatCommon
{

///////////////////////////////////////////////////////////////////////////////
/// UserCore - Основной класс реализующий бизнес-логику поиска ситуаций
///
class UserCore
    : public QObject
{
    Q_OBJECT

public:
    explicit UserCore(const HTTPClientConfig& httpClientConfig, const TradingCatCommon::KLineTypes& types = {TradingCatCommon::KLineType::MIN1}, QObject* parent = nullptr);

    /*!
        Деструктор
    */
    ~UserCore();

    TradingCatCommon::StockExchangesIDList stockExchangesIdList() const;

public slots:
    void start();
    void stop();

    //for Decector
    void setUserFilters(qint64 userId, const TradingCatCommon::Filter& filter);
    void eraseUserFilter(qint64 userId);

    void getServerStatus(qint64 userId);

signals:
    void sendLogMsg(Common::TDBLoger::MSG_CODE category, const QString& msg);

    void klineDetect(const TradingCatCommon::Detector::PKLineDetectData& klineData, const TradingCatCommon::PKLinesList& klinesList);

    void serverStatus(qint64 userId, const QString&serverName, const QString& serverVersion, const QDateTime& serverTime, qint64 upTime);

    void finished();

private slots:
    void klineDetectDetector(const TradingCatCommon::Detector::PKLineDetectData& klineData);

    void sendLogMsgHTTPClient(Common::TDBLoger::MSG_CODE category, const QString& msg);

    void stockExchangeListHTTPClient(const TradingCatCommon::StockExchangesIDList& stockExchangeIdList, quint64 id);
    void klineListHTTPClient(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::KLinesIDList& klineIdList, quint64 id);
    void klineNewHTTPClient(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klinesList, quint64 id);
    void klineHistoryHTTPClient(const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klinesList, quint64 id);
    void serverStatusHTTPClient(const QString&serverName, const QString& serverVersion, const QDateTime& serverTime, qint64 upTime, quint64 id);
    void errorOccurredHTTPClient(const QString& msg, TradingCatCommon::PackageType type, quint64 id);

    void updateNew();

private:
    UserCore() = delete;
    Q_DISABLE_COPY_MOVE(UserCore);

private:
    using StockExchangesMap = std::unordered_map<TradingCatCommon::StockExchangeID, TradingCatCommon::KLinesIDList>;

private:
    const HTTPClientConfig _httpClientConfig;

    TradingCatCommon::Detector* _detector = nullptr;
    TradingCatCommon::HTTPClient* _httpClient = nullptr;

    const TradingCatCommon::KLineTypes _types = {TradingCatCommon::KLineType::MIN1};

    QTimer* _updateTimer = nullptr;

    StockExchangesMap::const_iterator _nextStockExchange;
    StockExchangesMap _stockExchangesMap;

    std::unordered_map<quint64, TradingCatCommon::Detector::PKLineDetectData> _detectKLine;

    std::unordered_map<quint64, qint64> _serverStatus;

    bool _isStarted = false;
};

} //namespace TradingCatCommon
