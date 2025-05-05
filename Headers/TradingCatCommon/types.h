#pragma once

//STL
#include <list>

//Qt
#include <QHostAddress>
#include <QDateTime>
#include <QCoreApplication>

//My
#include "TradingCatCommon/kline.h"

namespace TradingCatCommon
{

static const size_t COUNT_SAVE_KLINES = 300;

struct ProxyData
{
    QHostAddress address;
    quint32 port = 0;
    QString user;
    QString password;
};

using ProxyDataList = std::list<ProxyData>;

struct StockExchangeConfig
{
    QString type;
    QString user;
    QString password;
    TradingCatCommon::KLineTypes klineTypes = {TradingCatCommon::KLineType::MIN1, TradingCatCommon::KLineType::MIN5};
    QStringList klineNames;
};

using StockExchangeConfigList = std::list<StockExchangeConfig>;

struct HTTPServerConfig
{
    QHostAddress address = QHostAddress::LocalHost;
    quint16 port = 80;
    quint64 maxUsers = 1000;
    QString rootDir = QCoreApplication::applicationDirPath();
    QString name;
    const QDateTime startDateTime = QDateTime::currentDateTime();
};

} //namespace TradingCatCommon
