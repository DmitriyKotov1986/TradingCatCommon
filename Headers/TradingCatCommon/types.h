#pragma once

//STL
#include <list>

//Qt
#include <QHostAddress>
#include <QDateTime>
#include <QCoreApplication>

//My
#include "TradingCatCommon/kline.h"

namespace TradingCatCommon
{

static const size_t COUNT_SAVE_KLINES = 300;  ///< Максимальное количество передаваемых свечей

///////////////////////////////////////////////////////////////////////////////
///     The ProxyData class - конфигурация прокси сервера
///
struct ProxyData
{
    QHostAddress address;   ///< Адрес
    quint32 port = 0;       ///< Порт
    QString user;           ///< Имя пользователя
    QString password;       ///< Пароль
};

using ProxyDataList = std::list<ProxyData>; ///< Список параметров прокси

///////////////////////////////////////////////////////////////////////////////
///     The StockExchangeConfig class - конфигурация подключения к бирже
///
struct StockExchangeConfig
{
    QString type;           ///< Тип биржи. Должен совпадать с названием биржи
    QString user;           ///< Имя пользователя или API key (зависит от биржи)
    QString password;       ///< Пароль или Secret key (зависит от биржи)
    TradingCatCommon::KLineTypes klineTypes = {TradingCatCommon::KLineType::MIN1, TradingCatCommon::KLineType::MIN5}; ///< Список интервалов свечей, которые необходимо получать с биржи
    QStringList klineNames; ///< Фильтр названия инструментов, которые нужно получать с биржи
};

using StockExchangeConfigList = std::list<StockExchangeConfig>;   ///< Список конфигураций бирж

///////////////////////////////////////////////////////////////////////////////
///     The HTTPServerConfig class - конфигурация HTTP сервера
///
struct HTTPServerConfig
{
    QHostAddress address = QHostAddress::LocalHost;                 ///< адрес интерфейса на котором запускаеться сервер
    quint16 port = 80;                                              ///< Порт
    quint64 maxUsers = 1000;                                        ///< Максимальное количество пользователей
    QString rootDir = QCoreApplication::applicationDirPath();       ///< Корневая папка
    QString name;                                                   ///< Название сервера
    const QDateTime startDateTime = QDateTime::currentDateTime();   ///< Время запуска
};

} //namespace TradingCatCommon
