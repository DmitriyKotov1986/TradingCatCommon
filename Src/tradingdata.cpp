//My
//#include "TradingCatCommon/dbloader.h"

#include "TradingCatCommon/tradingdata.h"

using namespace TradingCatCommon;
using namespace Common;

TradingData::TradingData(const TradingCatCommon::StockExchangesIDList& stockExcangesIdList, QObject* parent /* = nullptr */)
    : QObject{parent}
    , _stockExcangesIdList(stockExcangesIdList)
{
    qRegisterMetaType<TradingCatCommon::StockExchangeID>("TradingCatCommon::StockExchangeID");
    qRegisterMetaType<TradingCatCommon::PKLinesList>("TradingCatCommon::PKLinesList");
}

TradingData::~TradingData()
{
    stop();
}

void TradingData::start()
{
    Q_ASSERT(!_isStarted);

    _dataKLine = std::make_unique<KLinesDataContainer>(_stockExcangesIdList);

    _isStarted = true;
}

void TradingData::stop()
{
    if (!_isStarted)
    {
        emit finished();

        return;
    }

    _dataKLine.reset();

    _isStarted = false;

    emit finished();
}

TradingCatCommon::PKLinesList TradingData::getKLinesOnDate(const StockExchangeID &stockExchangeID, const KLineID &klineID, const qint64 start, const qint64 end) const
{
    Q_ASSERT(!stockExchangeID.isEmpty());
    Q_ASSERT(!klineID.isEmpty());
    Q_ASSERT(start != 0);
    Q_ASSERT(end != 0);
    Q_ASSERT(start < end);

    return _dataKLine->getKLinesOnDate(stockExchangeID, klineID, start, end);
}

qsizetype TradingData::moneyCount() const noexcept
{
    return _dataKLine->moneyCount();
}

const KLinesIDList &TradingData::getKLineList(const StockExchangeID &stockExchangeID) const
{
    Q_ASSERT(!stockExchangeID.isEmpty());

    return _dataKLine->getKLineList(stockExchangeID);
}

const StockExchangesIDList &TradingData::stockExcangesIdList() const noexcept
{
    return _stockExcangesIdList;
}

void TradingData::addKLines(const TradingCatCommon::StockExchangeID& stockExchangeID, const PKLinesList &klines)
{
    Q_ASSERT(!stockExchangeID.isEmpty());
    Q_ASSERT(!klines->empty());

    _dataKLine->addKLines(stockExchangeID, klines);
}


