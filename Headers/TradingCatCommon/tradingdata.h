#pragma once

//STL
#include <memory>

//Qt
#include <QObject>

//My
#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"
#include "TradingCatCommon/klinesdatacontainer.h"
#include "TradingCatCommon/transmitdata.h"

namespace TradingCatCommon
{

class TradingData
    : public QObject
{
    Q_OBJECT

public:
    explicit TradingData(const TradingCatCommon::StockExchangesIDList& stockExcangesIdList, QObject* parent = nullptr);

    /*!
        Деструктор
    */
    ~TradingData();

    //for HTTP server
    TradingCatCommon::PKLinesList getKLinesOnDate(const TradingCatCommon::StockExchangeID &stockExchangeID,
                                                  const TradingCatCommon::KLineID& klineID,
                                                  const QDateTime& start,
                                                  const QDateTime& end) const;

    qsizetype moneyCount() const noexcept;
    const TradingCatCommon::StockExchangesIDList& stockExcangesIdList() const noexcept;

signals:
    void finished();

public slots:
    void start();
    void stop();

    void addKLines(const TradingCatCommon::StockExchangeID& stockExchangeID, const TradingCatCommon::PKLinesList& klines);

private:
    TradingData() = delete;
    Q_DISABLE_COPY_MOVE(TradingData);

private:
    const TradingCatCommon::StockExchangesIDList _stockExcangesIdList;
    std::unique_ptr<TradingCatCommon::KLinesDataContainer> _dataKLine;

    bool _isStarted = false;

};

} // namespace TradingCatCommon
