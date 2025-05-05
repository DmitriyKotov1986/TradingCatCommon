#pragma once

//Qt
#include <QObject>

//My
#include <Common/common.h>
#include <Common/tdbloger.h>

#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"

#ifndef QT_NO_SSL

namespace TradingCatCommon
{

class IStockExchange
    : public QObject
{
    Q_OBJECT

public:
    IStockExchange(const TradingCatCommon::StockExchangeID& id, QObject* parent = nullptr);
    ~IStockExchange() = default;

    const TradingCatCommon::StockExchangeID& id() const;

public slots:
    virtual void start();
    virtual void stop();

signals:
    void getKLines(const TradingCatCommon::StockExchangeID& id, const TradingCatCommon::PKLinesList& klines);

    void errorOccurred(const TradingCatCommon::StockExchangeID& id, Common::EXIT_CODE errorCode, const QString& errorString);
    void sendLogMsg(const TradingCatCommon::StockExchangeID& id, Common::TDBLoger::MSG_CODE category, const QString& msg);

    void finished();

private:
    IStockExchange() = delete;
    Q_DISABLE_COPY_MOVE(IStockExchange);

private:
    const TradingCatCommon::StockExchangeID _id;

};

} //namespace TradingCatCommon

#endif
