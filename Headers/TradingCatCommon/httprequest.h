#pragma once

//QT
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QUrlQuery>
#include <QUrl>

namespace TradingCatCommon
{

class HTTPRequest final
{
public:
    enum class RequestType
    {
        UNDEFINE,
        GET,
        POST
    };

public:
    HTTPRequest() = default;

    void add(const QByteArray& data);

    RequestType type() const noexcept;         //возвращает тип запроса
    QByteArray body() const noexcept;          //возвращает тело запроса
    QString header(const QString& key) const;  //возвращает заголовок
    QUrl resurce() const noexcept;             //возввращает запрашиваемый ресурс
    QString protocol() const noexcept;         //название протокола (Обычно HTTP/1.1)
    [[nodiscard]] uint errorCode() const;      //код ошибки
    [[nodiscard]] QString errorMsg() const;    //текстовое сообщение об ошибке
    bool isComplite() const noexcept;          //возвращает истину если пришли все данные
    bool isGetHeader() const noexcept;         //возвращает истину если пришел заголовок и он разобран
    qint64 size() const noexcept;              //общий обем пришедших данных
    quint64 expectedSize() const noexcept;

private:
    RequestType _type  = RequestType::UNDEFINE;
    QUrl _resurce;
    QString _protocol;
    QByteArray _body;
    QMap<QString, QString> _header;
    mutable uint16_t _errorCode = 204;
    mutable QString _errorMsg;
    qint64 _lengthBody = 0;
    qint64 _totalGetSize = 0;
    qint64 _expectedSize = 0;
    bool _headerParsed = false;
};

} //TradingCatCommon

