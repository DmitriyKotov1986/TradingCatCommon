//STL
#include <algorithm>

//Qt
#include <QTimer>
#include <QRandomGenerator64>

#include "TradingCatCommon/klinehttppool.h"

#if !QT_NO_SSL

using namespace TradingCatCommon;

KLineHTTPPool::KLineHTTPPool(Common::HTTPSSLQuery* http, QObject* parent /* = nullptr */)
    : QObject{parent}
    , _http(http)
{
    Q_CHECK_PTR(_http);
}

KLineHTTPPool::~KLineHTTPPool()
{
    for (auto& kline:_klines)
    {
        kline.second->stop();
    }

    _klines.clear();
}

void KLineHTTPPool::setUserPassword(const QString &user, const QString &password)
{
    Q_CHECK_PTR(_http);

    _http->setUserPassword(user, password);
}

void KLineHTTPPool::addKLine(std::unique_ptr<IKLine> &&kline)
{
    Q_CHECK_PTR(kline);

    const auto id = kline->id();

    QObject::connect(kline.get(), SIGNAL(getKLines(const TradingCatCommon::PKLinesList&)),
                     SLOT(getKLinesKLine(const TradingCatCommon::PKLinesList&)));
    QObject::connect(kline.get(), SIGNAL(errorOccurred(const TradingCatCommon::KLineID&, Common::EXIT_CODE, const QString&)),
                     SLOT(errorOccurredKLine(const TradingCatCommon::KLineID&, Common::EXIT_CODE, const QString&)));
    QObject::connect(kline.get(), SIGNAL(sendLogMsg(const TradingCatCommon::KLineID&, Common::MSG_CODE, const QString&)),
                     SLOT(sendLogMsgKLine(const TradingCatCommon::KLineID&, Common::MSG_CODE, const QString&)));

    kline->setHTTP(_http);

    const auto p_kline = kline.get();
    QTimer::singleShot(QRandomGenerator64::global()->bounded(static_cast<qint64>(id.type) * 2), this, [p_kline](){ p_kline->start(); });

    _klines.emplace(std::move(id), std::move(kline));
}

void KLineHTTPPool::deleteKLine(const KLineID &id)
{
    const auto it_klines = _klines.find(id);

    Q_ASSERT(it_klines != _klines.end());

    it_klines->second->stop();

    _klines.erase(it_klines);
}

bool KLineHTTPPool::isExitsKLine(const KLineID& id) const
{
    return _klines.contains(id);
}

KLinesIDList KLineHTTPPool::addedKLines() const
{
    KLinesIDList result;
    for (const auto& [key, _]: _klines)
    {
        result.insert(key);
    }

    return result;
}

qsizetype KLineHTTPPool::klineCount() const noexcept
{
    return _klines.size();
}

void KLineHTTPPool::getKLinesKLine(const PKLinesList &klines)
{
    Q_ASSERT(!klines->empty());

    emit getKLines(klines);
}

void KLineHTTPPool::errorOccurredKLine(const TradingCatCommon::KLineID& id, Common::EXIT_CODE errorCode, const QString& errorString)
{
    emit errorOccurred(errorCode, QString("Error get KLine ID %1 from HTTP server: %2").arg(id.toString()).arg(errorString));
}

void KLineHTTPPool::sendLogMsgKLine(const TradingCatCommon::KLineID& id, Common::MSG_CODE category, const QString& msg)
{
    emit sendLogMsg(category, QString("KLine ID %1: %2").arg(id.toString()).arg(msg));
}

#endif
