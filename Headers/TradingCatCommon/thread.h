#pragma once

#include <QThread>
#include <QObject>

namespace TradingCatCommon
{

class Thread
    : public QThread
{
    Q_OBJECT

public:
    explicit Thread(quint64 id, QObject *parent = nullptr);
    ~Thread() {}

    quint64 id() const noexcept;

signals:
    void finished(quint64 id);
    void started(quint64 id);

private slots:
    void finished();
    void started();

private:
    const quint64 _id = 0;

}; //class Thread

} //namespace TradingCatCommon

