//Qt
#include <QMutex>
#include <QMutexLocker>

#include "TradingCatCommon/tradingdata.h"

using namespace TradingCatCommon;
using namespace Common;

Q_GLOBAL_STATIC(QMutex, klinesIDListMutex);

TradingData::TradingData(const TradingCatCommon::StockExchangesIDList& stockExcangesIdList, QObject* parent /* = nullptr */)
    : QObject{parent}
    , _stockExcangesIdList(stockExcangesIdList)
{
    qRegisterMetaType<TradingCatCommon::StockExchangeID>("TradingCatCommon::StockExchangeID");
    qRegisterMetaType<TradingCatCommon::PKLinesList>("TradingCatCommon::PKLinesList");
    qRegisterMetaType<TradingCatCommon::PKLinesIDList>("TradingCatCommon::PKLinesIDList");

    for (const auto& stockExcangesId: _stockExcangesIdList)
    {
        _klinesIdList.emplace(stockExcangesId, std::make_shared<KLinesIDList>());
    }
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
    _isSendStarted = false;
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

const PKLinesIDList &TradingData::getKLinesIDList(const StockExchangeID &stockExchangeID) const
{
    Q_ASSERT(!stockExchangeID.isEmpty());

    QMutexLocker<QMutex> klinesIDListLocker(klinesIDListMutex);

    return _klinesIdList.at(stockExchangeID);
}

const StockExchangesIDList &TradingData::stockExcangesIdList() const noexcept
{
    return _stockExcangesIdList;
}

void TradingData::addKLines(const TradingCatCommon::StockExchangeID& stockExchangeID, const PKLinesList& klines)
{
    Q_ASSERT(!stockExchangeID.isEmpty());
    Q_ASSERT(!klines->empty());

    _dataKLine->addKLines(stockExchangeID, klines);
}

void TradingData::getKLinesID(const StockExchangeID& stockExchangeId, const PKLinesIDList& klinesId)
{
    QMutexLocker<QMutex> klinesIDListLocker(klinesIDListMutex);

    auto& stokExchangeKLinesId = _klinesIdList.at(stockExchangeId);

    Q_CHECK_PTR(stokExchangeKLinesId);

    stokExchangeKLinesId.reset();

    stokExchangeKLinesId = klinesId;

    if (!_isSendStarted)
    {
        QStringList stList;
        for (const auto& st: _klinesIdList)
        {
            if (st.second->empty())
            {
                stList.push_back(st.first.name);
            }
        }
        if (!stList.isEmpty())
        {
            qDebug() << "Unget klines id list data from:" << stList.join(',');
        }

        const auto isAll = std::all_of(_klinesIdList.begin(), _klinesIdList.end(),
                                       [](const auto& item)
                                       {
                                           return !item.second->empty();
                                       });

        if (isAll)
        {
            emit started();

            _isSendStarted = true;
        }
    }
}


