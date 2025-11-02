#include "TradingCatCommon/ikline.h"

#if !QT_NO_SSL

using namespace TradingCatCommon;

IKLine::IKLine(const KLineID &id, QObject *parent /* = nullptr */)
    : QObject{parent}
    , _id(id)
{
    Q_ASSERT(!_id.isEmpty());

    qRegisterMetaType<TradingCatCommon::KLineID>("TradingCatCommon::KLineID");
}

const KLineID &IKLine::id() const
{
    return _id;
}

void IKLine::setHTTP(Common::HTTPSSLQuery* http)
{
    Q_CHECK_PTR(http);

    _http = http;
}

Common::HTTPSSLQuery *IKLine::getHTTP() const noexcept
{
    return _http;
}

void TradingCatCommon::IKLine::start()
{
    Q_CHECK_PTR(_http);
}

void TradingCatCommon::IKLine::stop()
{
}

void IKLine::addKLines(const PKLinesList &klines)
{
    if (!klines->empty())
    {
        emit getKLines(klines);
    }
}

#endif
