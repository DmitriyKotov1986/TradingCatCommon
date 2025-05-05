#include "TradingCatCommon/thread.h"

using namespace TradingCatCommon;

Thread::Thread(quint64 id, QObject *parent)
    : QThread{parent}
    , _id(id)
{
    QObject::connect(this, SIGNAL(finished()), SLOT(finished()));
    QObject::connect(this, SIGNAL(started()), SLOT(started()));
}

quint64 Thread::id() const noexcept
{
    return _id;
}

void Thread::finished()
{
    emit finished(_id);
}

void Thread::started()
{
    emit started(_id);
}
