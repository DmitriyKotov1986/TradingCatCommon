//Qt
#include <QtCharts/QtCharts>
#include <QtCharts/QValueAxis>

//My
#include <Common/common.h>

#include "TradingCatCommon/klinechart.h"

using namespace TradingCatCommon;

KLineChart::KLineChart()
{
    //Chart
    _series = new QCandlestickSeries();
    _series->setIncreasingColor(QColor(Qt::green));
    _series->setDecreasingColor(QColor(Qt::red));
    _series->setBodyOutlineVisible(true);
    _series->setBodyWidth(0.7);
    _series->setCapsVisible(true);
    _series->setCapsWidth(0.5);
    _series->setMinimumColumnWidth(-1.0);
    _series->setMaximumColumnWidth(50.0);
    auto pen = _series->pen();
    pen.setColor(Qt::white);
    _series->setPen(pen);

    _seriesVolume = new QCandlestickSeries();
    _seriesVolume->setIncreasingColor(QColor(61, 56, 70));
    _seriesVolume->setDecreasingColor(QColor(61, 56, 70));
    _seriesVolume->setBodyOutlineVisible(false);
    _seriesVolume->setCapsVisible(true);
    _seriesVolume->setMinimumColumnWidth(-1.0);
    _seriesVolume->setMaximumColumnWidth(50.0);
    _seriesVolume->setCapsWidth(0.5);

    auto chart = new QChart();
    chart->addSeries(_seriesVolume);
    chart->addSeries(_series);
    chart->setTitle("No data");
    //   chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundBrush(QBrush(QColor(36, 31, 49)));
    auto titleFont = chart->titleFont();
    titleFont.setPointSize(20);
    chart->setTitleFont(titleFont);
    chart->legend()->setVisible(false);
    chart->setTitle("No data");
    chart->setMargins({0,0,0,0});

    auto axisX = new QDateTimeAxis;
    axisX->setTickCount(3);
    axisX->setFormat("hh:mm");
    axisX->setGridLineVisible(false);
    axisX->setLabelsColor(Qt::white);
    auto axisXLabelsFont = axisX->labelsFont();
    axisXLabelsFont.setPixelSize(16);
    axisX->setLabelsFont(axisXLabelsFont);

    chart->addAxis(axisX, Qt::AlignBottom);

    _series->attachAxis(axisX);
    _seriesVolume->attachAxis(axisX);

    auto axisY = new QValueAxis();
    axisY->setGridLineVisible(true);
    axisY->setGridLineColor(QColor(94, 92, 100));
    auto axisYGridLinePen = axisY->gridLinePen();
    axisYGridLinePen.setStyle(Qt::DotLine);
    axisY->setGridLinePen(axisYGridLinePen);
    axisY->setLabelsColor(Qt::white);
    auto axisYLabelsFont = axisY->labelsFont();
    axisYLabelsFont.setPixelSize(16);
    axisY->setLabelsFont(axisYLabelsFont);

    chart->addAxis(axisY, Qt::AlignRight);

    _series->attachAxis(axisY);

    auto axisY2 = new QValueAxis();
    axisY2->setLabelsVisible(false);
    //axisY2->setLabelsColor(Qt::white);
    //auto axisY2LabelsFont = axisY2->labelsFont();
    //axisY2LabelsFont.setPixelSize(16);
    //axisY2->setLabelsFont(axisYLabelsFont);

    chart->addAxis(axisY2, Qt::AlignLeft);

    _seriesVolume->attachAxis(axisY2);

    _chartWidget = new QWidget();
    _chartView = new QChartView(chart, _chartWidget);
    _chartView->resize(QSize(1200, 800));
}

KLineChart::~KLineChart()
{
    delete _chartWidget;
}

bool KLineChart::makeChart(const QString &fileName, const StockExchangeID &stockExchangeId, const PKLinesList &klinesList)
{
    Q_CHECK_PTR(_series);
    Q_CHECK_PTR(_chartView);

    Q_ASSERT(!klinesList->empty());

    _series->clear();
    _seriesVolume->clear();

    _chartView->chart()->setTitle(QString("%1: %2 %3")
                                      .arg(stockExchangeId.name)
                                      .arg((*klinesList->begin())->id.symbol)
                                      .arg(KLineTypeToString((*klinesList->begin())->id.type)));

    double max = std::numeric_limits<double>::min();
    double min = std::numeric_limits<double>::max();
    double maxVolume = std::numeric_limits<double>::min();

    for (auto kline_it = klinesList->begin(); kline_it != klinesList->end(); ++kline_it)
    {
        auto candlestick = new QCandlestickSet((*kline_it)->closeTime.toMSecsSinceEpoch());
        candlestick->setHigh((*kline_it)->high);
        candlestick->setLow((*kline_it)->low);
        candlestick->setOpen((*kline_it)->open);
        candlestick->setClose(std::fabs((*kline_it)->open - (*kline_it)->close) < 0.00000001 ? (*kline_it)->close : (*kline_it)->close + 0.0005 * (*kline_it)->close);
        auto pen = _series->pen();
        if ((*kline_it)->volume < 1.0)
        {
            pen.setColor(QColor(Qt::gray));
        }
        else
        {
            pen.setColor((*kline_it)->open <= (*kline_it)->close ? QColor(Qt::green) : QColor(Qt::red));
        }
        candlestick->setPen(pen);

        _series->append(candlestick);

        auto candlestickVolume = new QCandlestickSet((*kline_it)->closeTime.toMSecsSinceEpoch());
        candlestickVolume->setOpen((*kline_it)->volume);
        candlestickVolume->setHigh((*kline_it)->volume);
        candlestickVolume->setLow(0);
        candlestickVolume->setClose(0);
        auto penVolume = _seriesVolume->pen();
        penVolume.setColor(QColor(61, 56, 70));
        candlestickVolume->setPen(penVolume);

        _seriesVolume->append(candlestickVolume);

        max = std::max(max, (*kline_it)->high);
        min = std::min(min, (*kline_it)->low);
        maxVolume = std::max(maxVolume, (*kline_it)->volume);
    }

    auto axisX = qobject_cast<QDateTimeAxis*>(_chartView->chart()->axes(Qt::Horizontal).at(0));
    axisX->setMax((*klinesList->begin())->closeTime.addMSecs(static_cast<qint64>((*klinesList->begin())->id.type) * 5));
    axisX->setMin((*klinesList->rbegin())->closeTime.addMSecs(-static_cast<qint64>((*klinesList->begin())->id.type)));
    axisX->setTickCount(5);

    auto axisY = qobject_cast<QValueAxis*>(_chartView->chart()->axes(Qt::Vertical).at(0));
    axisY->setMax(max + max * 0.005);
    axisY->setMin(min - min * 0.005);

    auto axisY2 = qobject_cast<QValueAxis*>(_chartView->chart()->axes(Qt::Vertical).at(1));
    axisY2->setMax(maxVolume * 2);
    axisY2->setMin(0);

    Common::makeFilePath(fileName);

    _chartWidget->grab().save(fileName);

    return true;
}


