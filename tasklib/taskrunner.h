#ifndef TASKRUNNER_H
#define TASKRUNNER_H

#include <QFuture>
#include <QFutureWatcher>
#include <QThreadPool>
#include <functional>
#include <type_traits>

namespace TaskRunner {

class CancelToken
{
    QFutureInterfaceBase *m_future;

    friend class CancelWatcher;

public:
    Q_DISABLE_COPY_MOVE(CancelToken);
    explicit CancelToken(QFutureInterfaceBase *f) : m_future(f) {}


    bool isCanceled() const {
        return m_future->isCanceled();
    }
};

class CancelWatcher : public QObject
{
    Q_OBJECT
public:
    explicit CancelWatcher(QObject *parent = nullptr);

    bool isCanceled() const {
        return m_futureWatcher.isCanceled();
    }

    void setToken(const CancelToken &token);

signals:
    void canceled();

private:
    QFutureWatcher<void> m_futureWatcher;
};

// NOTE: use TFunctor&& and std::forward to support move-only functors

template <typename TFunctor, typename T>
class InternalRunTask : public QFutureInterface<T>, public QRunnable
{
    TFunctor m_func;
public:
    InternalRunTask(TFunctor &&func) : m_func(std::forward<TFunctor>(func)) {};

    QFuture<T> start()
    {
        this->setRunnable(this);
        this->reportStarted();
        QFuture<T> future = this->future();
        QThreadPool::globalInstance()->start(this);
        return future;
    }

    void run() override
    {
        if (this->isCanceled()) {
            this->reportFinished();
            return;
        }

        CancelToken cancelToken(this);
        this->reportResult(m_func(cancelToken));

        this->reportFinished();
    }
};

template <typename TFunctor, typename TResult = typename std::result_of<TFunctor&&(const CancelToken&)>::type>
QFuture<TResult> run(TFunctor &&func)
{
    return (new InternalRunTask<TFunctor, TResult>(std::forward<TFunctor>(func)))->start();
}

} // namespace TaskRunner

#endif // TASKRUNNER_H
