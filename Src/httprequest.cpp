//Qt
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>

#include "TradingCatCommon/httprequest.h"

using namespace TradingCatCommon;

void HTTPRequest::add(const QByteArray& data)
{
    _totalGetSize += data.size(); //суммируем объем принятых данных

    _body += data; //сохраняем пришедшие данные
    //проверяем пришел ли весь заголовок (должна содержаться пустая строчка)
    if (!_headerParsed)
    {
        if (_body.contains("\r\n\r\n")) { //найден заголовок - парсим его
            QTextStream requestStr(_body);

            requestStr.setAutoDetectUnicode(true); //преобразуем в UNICODE

            QString tmp;
            requestStr >> tmp; //читаем тип запроса
            if (tmp == "GET")
            {
                _type = RequestType::GET;
            }
            else if (tmp == "POST")
            {
                _type = RequestType::POST;
            }
            else
            {
                _errorCode = 405; //Bad request
                _errorMsg = "Method must be POST";

                return;
            }

            requestStr >> tmp; //читаем запрашиваемый ресурс
            tmp.remove(0,1);

            _resurce = tmp; //main.http

            requestStr >> tmp; //читаем версию протокола
            _protocol = tmp;      //HTTP/1.1
            if (_protocol != "HTTP/1.1") {
                _errorCode = 505; //Bad request
                _errorMsg = "Protocol must be HTTP/1.1";

                return;
            }

            requestStr.readLine(); //пропускаем строчку

            //считываем заголовок
            while (!requestStr.atEnd()) {
                QString key = requestStr.readLine();

                if (key == "")
                {
                    break; //дальше идет тело запроса
                }

                qsizetype pos = key.indexOf(":");
                QString value = key.mid(pos + 2); //читаем строку до конца

                key.remove(pos, key.length() - pos);

                _header.insert(key, value);
            }

            _expectedSize = _body.size();

            //остальное записываем в тело запроса
            _body.clear();
            _body = requestStr.readAll().toUtf8(); //все остальное - тело запроса

            bool ok = false;
            _lengthBody = header("Content-Length").toLongLong(&ok);
            if (_type == RequestType::POST && !ok)
            {
                _errorCode = 411; //Bad request
                _errorMsg = "Length Required";

                return;
            }

            _expectedSize = _expectedSize - _body.size() + _lengthBody;
            _headerParsed = true;
        }
    }
    if (isComplite() && isGetHeader())
    {
        _errorCode = 200;
    }
}

HTTPRequest::RequestType HTTPRequest::type() const noexcept
{
    return _type;
}

QByteArray HTTPRequest::body() const noexcept
{
    return _body;
}

QString HTTPRequest::header(const QString& key) const
{
    const auto header_it = _header.find(key);
    if (header_it == _header.end())
    {
        return QString();
    }

    return header_it.value();
}

QUrl HTTPRequest::resurce() const noexcept
{
    return _resurce;
}

QString HTTPRequest::protocol() const noexcept
{
    return _protocol;
}

uint HTTPRequest::errorCode() const
{
    const auto errorCode = _errorCode;
    _errorCode = 0;

    return errorCode;
}

QString HTTPRequest::errorMsg() const
{
    const auto errorMsg = _errorMsg;
    _errorMsg.clear();

    return errorMsg;
}

bool HTTPRequest::isComplite() const noexcept
{
    return (_body.size() >= _lengthBody);
}

bool HTTPRequest::isGetHeader() const noexcept
{
    return _headerParsed;
}

qint64 HTTPRequest::size() const noexcept
{
    return _totalGetSize;
}

quint64 HTTPRequest::expectedSize() const  noexcept
{
    return _expectedSize;
}

