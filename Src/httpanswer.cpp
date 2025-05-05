#include "TradingCatCommon/httpanswer.h"

using namespace TradingCatCommon;

HTTPAnswer::HTTPAnswer(uint16_t code)
{
    Q_ASSERT(AnswerCode.contains(code));

    _status = QString("HTTP/1.1 %1 %2").arg(code).arg(AnswerCode.value(code));
}

HTTPAnswer::~HTTPAnswer()
{
}

QByteArray HTTPAnswer::getAnswer()
{
    QByteArray answer = QString("%1\r\n").arg(_status).toUtf8();

    //добавляем тег Content-Length если его нет
    if (_headers.find("Content-Length") == _headers.end())
    {
        _headers.insert("Content-Length", QString::number(_body.size()));
    }
    if (_headers.find("Content-Type") == _headers.end())
    {
         _headers.insert("Content-Type", "application/json");
    }
    for (auto headers_it = _headers.begin(); headers_it != _headers.end(); ++headers_it)
    {
        answer += QString("%1: %2\r\n").arg(headers_it.key()).arg(headers_it.value()).toUtf8();
    }

    answer += "\r\n";
    answer += _body;

    return answer;
}

void HTTPAnswer::addHeader(const QString& key, const QString& value)
{
    _headers.insert(key,value);
}

void HTTPAnswer::addBody(const QByteArray& data)
{
    _body += data;
}
