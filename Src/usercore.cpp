//Qt
#include <QTimer>

#include "TradingCatCommon/usercore.h"

using namespace TradingCatCommon;

static const quint64 SEND_TIMEOUT = 1u; //10s
static const quint64 RETRY_SEND_TIMEOUT = 10000u; //10s
static const quint64 SEND_UPDATE_INTERVAL = 20u * 1000u; //10s


UserCore::UserCore(const HTTPClientConfig& httpClientConfig, const TradingCatCommon::KLineTypes& types /* = {TradingCatCommon::KLineType::MIN1} */, QObject* parent /* = nullptr */)
    : QObject{parent}
    , _httpClientConfig(httpClientConfig)\
    , _types(types)
{
    Q_ASSERT(_httpClientConfig.isCheck());
    Q_ASSERT(!_types.empty());

    qRegisterMetaType<TradingCatCommon::Detector::ChatsIdList>("TradingCatCommon::Detector::ChatsIdList");
    qRegisterMetaType<TradingCatCommon::Filter>("TradingCatCommon::Filter");
    qRegisterMetaType<TradingCatCommon::StockExchangeID>("TradingCatCommon::StockExchangeID");
    qRegisterMetaType<TradingCatCommon::KLineID>("TradingCatCommon::KLineID");
    qRegisterMetaType<TradingCatCommon::Filter::FilterTypes>("TradingCatCommon::Filter::FilterTypes");
    qRegisterMetaType<TradingCatCommon::PKLinesList>("TradingCatCommon::PKLinesList");
    qRegisterMetaType<TradingCatCommon::Detector::PKLineDetectData>("TradingCatCommon::Detector::PKLineDetectData");
}

UserCore::~UserCore()
{
    stop();
}

StockExchangesIDList UserCore::stockExchangesIdList() const
{
    // StockExchangesIDList result;
    // result.resize(_stockExchangesMap.size());

    // std::transform(_stockExchangesMap.begin(), _stockExchangesMap.end(), result.begin(),
    //     [](const auto& item)
    //     {
    //         return item.first;
    //     });

    // return result;
}

void UserCore::start()
{
    Q_ASSERT(!_isStarted);

    _detector = new Detector();

    connect(_detector, SIGNAL(klineDetect(const TradingCatCommon::Detector::PKLineDetectData&)),
            SLOT(klineDetectDetector(const TradingCatCommon::Detector::PKLineDetectData&)));

    connect(_detector, SIGNAL(orderDetect(const TradingCatCommon::Detector::POrderDetectData&)),
            SLOT(orderDetectDetector(const TradingCatCommon::Detector::POrderDetectData&)));

    _httpClient = new HTTPClient(_httpClientConfig);

    connect(_httpClient, SIGNAL(stockExchangeList(const TradingCatCommon::StockExchangesIDList&, quint64)),
            SLOT(stockExchangeListHTTPClient(const TradingCatCommon::StockExchangesIDList&, quint64)));

    connect(_httpClient, SIGNAL(klineList(const TradingCatCommon::StockExchangeID&, const TradingCatCommon::KLinesIDList&, quint64)),
            SLOT(klineListHTTPClient(const TradingCatCommon::StockExchangeID&, const TradingCatCommon::KLinesIDList&, quint64)));

    connect(_httpClient, SIGNAL(klineNew(const TradingCatCommon::StockExchangeID&, const TradingCatCommon::PKLinesList&, quint64)),
            SLOT(klineNewHTTPClient(const TradingCatCommon::StockExchangeID&, const TradingCatCommon::PKLinesList&, quint64)));

    connect(_httpClient, SIGNAL(orderBookNew(const TradingCatCommon::StockExchangeID&, const TradingCatCommon::POrderBooksList&, quint64)),
            SLOT(orderBookNewHTTPClient(const TradingCatCommon::StockExchangeID&, const TradingCatCommon::POrderBooksList&, quint64)));

    connect(_httpClient, SIGNAL(klineHistory(const TradingCatCommon::StockExchangeID&, const TradingCatCommon::PKLinesList&, quint64)),
            SLOT(klineHistoryHTTPClient(const TradingCatCommon::StockExchangeID&, const TradingCatCommon::PKLinesList&, quint64)));

    connect(_httpClient, SIGNAL(serverStatus(const QString&, const QString&, const QDateTime&, qint64, quint64)),
            SLOT(serverStatusHTTPClient(const QString&, const QString&, const QDateTime&, qint64, quint64)));

    connect(_httpClient, SIGNAL(errorOccurred(const QString&, TradingCatCommon::PackageType, quint64)),
            SLOT(errorOccurredHTTPClient(const QString&, TradingCatCommon::PackageType, quint64)));

    connect(_httpClient, SIGNAL(sendLogMsg(Common::TDBLoger::MSG_CODE, const QString&)),
            SLOT(sendLogMsgHTTPClient(Common::TDBLoger::MSG_CODE, const QString&)));

    _isStarted = true;

    _httpClient->sendStockExchangeList();
}

void UserCore::stop()
{
    if (!_isStarted)
    {
        return;
    }

    delete _updateTimer;
    _updateTimer = nullptr;

    delete _detector;
    _detector = nullptr;

    delete _httpClient;
    _httpClient = nullptr;

    _isStarted = false;

    emit finished();
}

void UserCore::setUserFilters(qint64 userId, const Filter& filter)
{
    Q_CHECK_PTR(_detector);

    Q_ASSERT(_isStarted);

    _detector->setUserFilters(userId, filter);
}

void UserCore::eraseUserFilter(qint64 userId)
{
    Q_CHECK_PTR(_detector);

    Q_ASSERT(_isStarted);

    _detector->eraseUserFilter(userId);
}

void UserCore::getServerStatus(qint64 userId)
{
    Q_CHECK_PTR(_httpClient);

    const auto id = _httpClient->sendServerStatus();

    _serverStatus.emplace(id, userId);
}

void UserCore::klineDetectDetector(const TradingCatCommon::Detector::PKLineDetectData& klineData)
{
    const auto curDateTime = QDateTime::currentDateTime().addSecs(60);
    const auto startTime = curDateTime.addMSecs(-(static_cast<qint64>(klineData->klineId.type) * 60));
    auto id = _httpClient->sendKLineHistory(klineData->stockExchangeId, klineData->klineId, startTime, curDateTime);

    _detectKLine.emplace(std::move(id), klineData);
}

void UserCore::sendLogMsgHTTPClient(Common::TDBLoger::MSG_CODE category, const QString &msg)
{
    emit sendLogMsg(category, QString("HTTP client of stock exchange server: %1").arg(msg));
}

void UserCore::stockExchangeListHTTPClient(const StockExchangesIDList &stockExchangeIdList, quint64 id)
{
    Q_ASSERT(id != 0);

    _stockExchangesMap.clear();

    for (const auto& stockExchangeId: stockExchangeIdList)
    {
        _stockExchangesMap.emplace(stockExchangeId, KLinesIDList{});
    }

    if (_stockExchangesMap.empty())
    {
        emit sendLogMsg(Common::TDBLoger::MSG_CODE::WARNING_CODE, "The server did not report supported exchanges. Retry...");

        QTimer::singleShot(RETRY_SEND_TIMEOUT, this, [this](){ _httpClient->sendStockExchangeList(); });

        return;
    }

    _nextStockExchange = _stockExchangesMap.begin();

    QTimer::singleShot(SEND_TIMEOUT, this, [this](){ _httpClient->sendKLineList(_nextStockExchange->first); });
}

void UserCore::klineListHTTPClient(const StockExchangeID &stockExchangeId, const KLinesIDList &klineIdList, quint64 id)
{
    // Q_ASSERT(id != 0);
    // Q_ASSERT(!stockExchangeId.isEmpty());

    // const auto it_stockExchangeMap = _stockExchangesMap.find(stockExchangeId);

    // if (it_stockExchangeMap == _stockExchangesMap.end())
    // {
    //     emit sendLogMsg(Common::TDBLoger::MSG_CODE::WARNING_CODE, QString("Invalid server response. Stock exchange %1 is missing from the request StockExchangeList. Retry...").arg(stockExchangeId.toString()));

    //     QTimer::singleShot(RETRY_SEND_TIMEOUT, this, [this](){ _httpClient->sendStockExchangeList(); });

    //     return;
    // }

    // auto& klinesIdList = it_stockExchangeMap->second;

    // for (const auto& klineId: klineIdList)
    // {
    //     klinesIdList.insert(klineId);
    // }

    // _nextStockExchange = std::next(_nextStockExchange);

    // if (_nextStockExchange != _stockExchangesMap.end())
    // {
    //     QTimer::singleShot(SEND_TIMEOUT, this, [this](){ _httpClient->sendKLineList(_nextStockExchange->first); });
    // }
    // else
    // {
    //     quint64 klineIdCount = 0;
    //     for (const auto& stockExchange: _stockExchangesMap)
    //     {
    //         klineIdCount += stockExchange.second.size();
    //     }

    //     emit sendLogMsg(Common::TDBLoger::MSG_CODE::INFORMATION_CODE, QString("Stock exchange data successfully received. Total stock exchanges: %1, klines ID: %2. Start get new klines")
    //                                                                       .arg(_stockExchangesMap.size())
    //                                                                       .arg(klineIdCount));

    //     if (!_updateTimer)
    //     {
    //         _updateTimer = new QTimer();

    //         connect(_updateTimer, SIGNAL(timeout()), SLOT(updateNew()));

    //         _httpClient->sendKLineNew(_types);

    //         _updateTimer->start(SEND_UPDATE_INTERVAL);
    //     }
    // }
}

void UserCore::klineNewHTTPClient(const StockExchangeID &stockExchangeId, const PKLinesList &klinesList, quint64 id)
{
    Q_CHECK_PTR(_httpClient);

    Q_ASSERT(id != 0);
    Q_ASSERT(!stockExchangeId.isEmpty());

    const auto curDateTime = QDateTime::currentDateTime().addSecs(60);
    quint64 index = 1;
    for (const auto& kline: *klinesList)
    {
        const auto& id = kline->id;

    }

    _detector->getNewKLine(stockExchangeId, klinesList);
}

void UserCore::klineHistoryHTTPClient(const StockExchangeID &stockExchangeId, const PKLinesList &klinesList, quint64 id)
{
    Q_ASSERT(id != 0);
    Q_ASSERT(!stockExchangeId.isEmpty());
    Q_ASSERT(!klinesList->empty());

    _detector->getKLineHistory(stockExchangeId, klinesList);

    const auto it_detectKLine = _detectKLine.find(id);
    if (it_detectKLine != _detectKLine.end())
    {
        const auto& detectklineInfo = it_detectKLine->second;
        emit klineDetect(detectklineInfo, klinesList);

        _detectKLine.erase(it_detectKLine);

        return;
    }
}

void UserCore::serverStatusHTTPClient(const QString &serverName, const QString &serverVersion, const QDateTime &serverTime, qint64 upTime, quint64 id)
{
    Q_ASSERT(id != 0);

    const auto it_serverStatus = _serverStatus.find(id);
    if (it_serverStatus == _serverStatus.end())
    {
        return;
    }

    emit serverStatus(it_serverStatus->second, serverName, serverVersion, serverTime, upTime);
}

void UserCore::errorOccurredHTTPClient(const QString &msg, TradingCatCommon::PackageType type, quint64 id)
{
    Q_ASSERT(id != 0);

    QString packageTypeStr;
    QString action("Retry..");

    switch (type)
    {
    case PackageType::STOCK_EXCHANGE_LIST:
    {
        packageTypeStr = "StockExchangeList";
        QTimer::singleShot(RETRY_SEND_TIMEOUT, this, [this](){ _httpClient->sendStockExchangeList(); });
        break;
    }
    case PackageType::KLINE_LIST:
    {
        packageTypeStr = "KLineList";
        QTimer::singleShot(RETRY_SEND_TIMEOUT, this, [this](){ _httpClient->sendKLineList(_nextStockExchange->first); });
        break;
    }
    case PackageType::KLINE_NEW:
    {
        packageTypeStr = "KLineNew";
        action = "Ignore..";
        break;
    }
    case PackageType::KLINE_HISTORY:
    {
        packageTypeStr = "KLineHistory";
        action = "Ignore..";

        const auto it_deetectKLine = _detectKLine.find(id);
        if (it_deetectKLine != _detectKLine.end())
        {
            _detectKLine.erase(it_deetectKLine);
        }
        break;
    }  
    case PackageType::SERVER_STATUS:
    {
        packageTypeStr = "ServerStatus";
        action = "Ignore..";

        _serverStatus.erase(id);

        break;
    }
    default:
        packageTypeStr = "UNDEFINE";
        break;
    }

    emit sendLogMsg(Common::TDBLoger::MSG_CODE::WARNING_CODE, QString("Error get data %1 from server. Package ID: %2. Error: %3. %4")
                                                                  .arg(packageTypeStr)
                                                                  .arg(id)
                                                                  .arg(msg)
                                                                  .arg(action));
}

void UserCore::updateNew()
{
    _httpClient->sendKLineNew(_types);
}


