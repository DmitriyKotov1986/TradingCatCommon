//STL
#include <cfloat>

//Qt

//My
#include "TradingCatCommon/kline.h"

#include "TradingCatCommon/transmitdata.h"

using namespace TradingCatCommon;
using namespace Common;

///////////////////////////////////////////////////////////////////////////////
///     Status
///
QString StatusAnswer::errorCodeToStr(ErrorCode code)
{
    switch (code)
    {
    case ErrorCode::OK: return "OK";
    case ErrorCode::BAD_REQUEST:return "Bad request";
    case ErrorCode::REQUEST_TIMEOUT: return "Request timeout";
    case ErrorCode::UNAUTHORIZED: return "Unauthorized";
    case ErrorCode::NOT_FOUND: return "Not found";
    case ErrorCode::INTERNAL_ERROR:return "Internal server error";
    case ErrorCode::UNDEFINED:
    default: return "Undefined";
    }

    return "Undefined";
}

StatusAnswer::ErrorCode StatusAnswer::intToErrorCode(quint16 code)
{
    switch (code)
    {
    case static_cast<quint16>(ErrorCode::OK): return ErrorCode::OK;
    case static_cast<quint16>(ErrorCode::BAD_REQUEST):return ErrorCode::BAD_REQUEST;
    case static_cast<quint16>(ErrorCode::REQUEST_TIMEOUT): return ErrorCode::REQUEST_TIMEOUT;
    case static_cast<quint16>(ErrorCode::UNAUTHORIZED): return ErrorCode::UNAUTHORIZED;
    case static_cast<quint16>(ErrorCode::NOT_FOUND): return ErrorCode::NOT_FOUND;
    case static_cast<quint16>(ErrorCode::INTERNAL_ERROR): return ErrorCode::INTERNAL_ERROR;
    case static_cast<quint16>(ErrorCode::UNDEFINED):
    default: return ErrorCode::UNDEFINED;
    }

    return ErrorCode::UNDEFINED;
}

StatusAnswer::StatusAnswer(const QJsonObject &json)
{ 
    const auto codeNum = JSONReadMapNumber<quint16>(json, "Code", "Root/Status/Code");

    _code = intToErrorCode(codeNum.has_value() ? codeNum.value() : static_cast<quint16>(ErrorCode::UNDEFINED));
    if (_code != ErrorCode::OK)
    {
        const auto o_msg = JSONReadMapString(json, "Msg", "Root/Status/Message");
        _msg = o_msg.has_value() ? o_msg.value() : "";
    }
}

StatusAnswer::StatusAnswer(ErrorCode code, const QString &msg /* = QString() */)
    : _code(code)
    , _msg(msg.isEmpty() ? errorCodeToStr(_code) : msg)
{
}

QJsonObject StatusAnswer::toJson() const
{
    QJsonObject result;

    result.insert("Code", static_cast<quint16>(_code));

    if (_code != ErrorCode::OK)
    {
        result.insert("Msg", _msg);
    }

    return result;
}

StatusAnswer::operator bool() const
{
    return _code == ErrorCode::OK;
}

const QString &StatusAnswer::message() const noexcept
{
    return _msg;
}

StatusAnswer::ErrorCode StatusAnswer::code() const noexcept
{
    return _code;
}

QString StatusAnswer::status() const
{
    return errorCodeToStr(_code);
}

///////////////////////////////////////////////////////////////////////////////
///     The class KLineIDJson
///
KLineIDJson::KLineIDJson(const KLineID &klineId)
    : _klineId(klineId)
{
}

KLineIDJson::KLineIDJson(const QJsonValue &json)
{
    const auto objJson = JSONReadMap(json, "StockExchangeID");
    const auto symbol = JSONReadMapString(objJson, "Symbol", "Symbol", false).value_or("");
    const auto type = stringToKLineType(JSONReadMapString(objJson, "Type", "Type", false).value_or(KLineTypeToString(KLineType::UNDEFINED)));

    _klineId = KLineID(symbol, type);
}

QJsonObject KLineIDJson::toJson() const
{
    QJsonObject result;

    result.insert("Symbol", _klineId.symbol);
    result.insert("Type", KLineTypeToString(_klineId.type));

    return result;
}

bool KLineIDJson::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &KLineIDJson::errorString() const noexcept
{
    return _errorString;
}

const KLineID& KLineIDJson::klineId() const noexcept
{
    return _klineId;
}

///////////////////////////////////////////////////////////////////////////////
///     The class StockExchangeIDJson
///
StockExchangeIDJson::StockExchangeIDJson(const StockExchangeID &stockExchangeId)
    : _stockExchangeId(stockExchangeId)
{
}

StockExchangeIDJson::StockExchangeIDJson(const QJsonValue &json)
{
    const auto objJson = JSONReadMap(json, "StockExchangeID");
    const auto name = JSONReadMapString(objJson, "Name", "StockExchange/Name", false).value_or("");

    _stockExchangeId = StockExchangeID(name);
}

QJsonObject StockExchangeIDJson::toJson() const
{
    QJsonObject result;

    result.insert("Name", _stockExchangeId.name);

    return result;
}

const StockExchangeID &StockExchangeIDJson::stockExchangeId() const noexcept
{
    return _stockExchangeId;
}

bool StockExchangeIDJson::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &StockExchangeIDJson::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The Query class базовй класс запроса
///
static quint64 queryId = 0;

Query::Query(PackageType type, const QString &path)
    : _type(type)
    , _path(path)
{
    Q_ASSERT(_type != PackageType::UNDEFINED);
    Q_ASSERT(!path.isEmpty());

    _id = ++queryId;
}

Query::~Query()
{
}

PackageType Query::type() const noexcept
{
    return _type;
}

const QString &Query::path() const noexcept
{
    return _path;
}

QUrlQuery Query::query() const
{
    return QUrlQuery();
}

quint64 Query::id() const noexcept
{
    return _id;
}

void Query::setID(quint64 id) noexcept
{
    Q_ASSERT(id);

    _id = id;
}

///////////////////////////////////////////////////////////////////////////////
///     The StockExchangesIDArrayJson class
///
StockExchangesIDArrayJson::StockExchangesIDArrayJson(const QJsonValue &json)
{
    try
    {
        const auto arrayJson = JSONReadArray(json, "StockExchangesIDList");

        for (const auto& stockExchangeIDJson: arrayJson)
        {
            StockExchangeIDJson stockExchangeID(stockExchangeIDJson);

            if (stockExchangeID.isError())
            {
                throw ParseException(stockExchangeID.errorString());
            }

            _stockExchangesIDList.insert(stockExchangeID.stockExchangeId());
        }
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();

        _stockExchangesIDList.clear();
    }
}

StockExchangesIDArrayJson::StockExchangesIDArrayJson(const StockExchangesIDList &stockExchangesIDList)
    : _stockExchangesIDList(stockExchangesIDList)
{
}

QJsonArray StockExchangesIDArrayJson::toJson() const
{
    QJsonArray result;

    for (const auto& stockExchangeId: _stockExchangesIDList)
    {
        StockExchangeIDJson stockExchangeIdJson(stockExchangeId);
        result.push_back(stockExchangeIdJson.toJson());
    }

    return result;
}

StockExchangesIDList StockExchangesIDArrayJson::stockExchagesIDList() const
{
    return _stockExchangesIDList;
}

bool StockExchangesIDArrayJson::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &StockExchangesIDArrayJson::errorString() const noexcept
{
    return _errorString;
}

///////////////////////////////////////////////////////////////////////////////
///     The KLineJson class
///
KLineJson::KLineJson(const PKLine &kline)
    : _kline(kline)
{
    Q_CHECK_PTR(_kline);
}

KLineJson::KLineJson(const QJsonValue &json)
{
    try
    {
        const auto klineJson = JSONReadMap(json, "KLine");

        _kline = std::make_shared<KLine>();

        _kline->openTime = JSONReadMapNumber<qint64>(klineJson, "OT", "KLine/OT", 0).value_or(0);
        _kline->open = JSONReadMapNumber<double>(klineJson, "O", "KLine/O", 0.0f).value_or(-1.0f);
        _kline->high = JSONReadMapNumber<double>(klineJson, "H", "KLine/H", 0.0f).value_or(-1.0f);
        _kline->low = JSONReadMapNumber<double>(klineJson, "L", "KLine/L", 0.0f).value_or(-1.0f);
        _kline->close = JSONReadMapNumber<double>(klineJson, "C", "KLine/C", 0.0f).value_or(-1.0f);
        _kline->volume = JSONReadMapNumber<double>(klineJson, "V", "KLine/V", 0.0f).value_or(-1.0f);
        _kline->closeTime = JSONReadMapNumber<qint64>(klineJson, "CT", "KLine/CT", 0).value_or(0);
        _kline->quoteAssetVolume = JSONReadMapNumber<double>(klineJson, "QAV", "KLine/QAV", 0.0f).value_or(-1.0f);

        const auto klineIdJson = JSONReadMapToMap(klineJson, "ID", "KLine/ID");
        _kline->id = KLineIDJson(klineIdJson).klineId();

        if (!_kline->check())
        {
            throw ParseException(QString("Incorrect value"));
        }
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
    }
}

QJsonObject KLineJson::toJson() const
{
    QJsonObject klineJson;

    klineJson.insert("ID", KLineIDJson(_kline->id).toJson());
    klineJson.insert("OT", static_cast<qint64>(_kline->openTime));
    klineJson.insert("O", _kline->open);
    klineJson.insert("H", _kline->high);
    klineJson.insert("L", _kline->low);
    klineJson.insert("C", _kline->close);
    klineJson.insert("V", _kline->volume);
    klineJson.insert("CT", static_cast<qint64>(_kline->closeTime));
    klineJson.insert("QAV", _kline->quoteAssetVolume);

    return klineJson;
}

bool KLineJson::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &KLineJson::errorString() const noexcept
{
    return _errorString;
}

const PKLine &KLineJson::kline() const noexcept
{
    return _kline;
}

///////////////////////////////////////////////////////////////////////////////
///     The KLinesArrayJson class - список свечей
///
KLinesArrayJson::KLinesArrayJson(const PKLinesList &klinesList)
    : _klinesList(klinesList)
{
    Q_CHECK_PTR(klinesList);
}

KLinesArrayJson::KLinesArrayJson(const QJsonValue &json)
{
    try
    {
        const auto klinesListJson = JSONReadArray(json, "KLine");

        _klinesList = std::make_shared<KLinesList>();

        for (const auto& klineJson: klinesListJson)
        {
            KLineJson kline(klineJson);
            if (kline.isError())
            {
                throw ParseException(QString("Incorrect KLine: ").arg(kline.errorString()));
            }

            _klinesList->emplace_back(kline.kline());
        }
    }
    catch (const ParseException& err)
    {
        _errorString = err.what();
    }
}

QJsonArray KLinesArrayJson::toJson() const
{
    QJsonArray result;

    for (const auto& kline: *_klinesList)
    {
        result.push_back(KLineJson(kline).toJson());
    }

    return result;
}

const PKLinesList &KLinesArrayJson::klinesList() const noexcept
{
    return _klinesList;
}

bool KLinesArrayJson::isError() const noexcept
{
    return !_errorString.isEmpty();
}

const QString &KLinesArrayJson::errorString() const noexcept
{
    return _errorString;
}
