#pragma once

//Qt
#include <QtCharts/QCandlestickSeries>
#include <QtCharts/QChartView>

//My
#include "TradingCatCommon/kline.h"
#include "TradingCatCommon/stockexchange.h"

namespace TradingCatCommon
{

class KLineChart
{
public:
    KLineChart();
    ~KLineChart();

    bool makeChart(const QString& fileName, const TradingCatCommon::StockExchangeID& stockExchangeId, const TradingCatCommon::PKLinesList& klinesList);

private:
    Q_DISABLE_COPY_MOVE(KLineChart);

    QWidget* _chartWidget = nullptr;
    QChartView* _chartView = nullptr;

    QCandlestickSeries* _series = nullptr;
    QCandlestickSeries* _seriesVolume = nullptr;

};


}
