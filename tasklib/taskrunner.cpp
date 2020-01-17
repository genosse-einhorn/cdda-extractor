#include "taskrunner.h"


TaskRunner::CancelWatcher::CancelWatcher(QObject *parent) : QObject(parent)
{
    connect(&m_futureWatcher, &QFutureWatcherBase::canceled, this, &CancelWatcher::canceled);
}

void TaskRunner::CancelWatcher::setToken(const TaskRunner::CancelToken &token)
{
    m_futureWatcher.setFuture(QFuture<void>(token.m_future));
}
