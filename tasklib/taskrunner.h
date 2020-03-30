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

    Q_DISABLE_COPY_MOVE(CancelToken);

public:
    explicit CancelToken(QFutureInterfaceBase *f) : m_future(f) {}


    bool isCanceled() const {
        return m_future->isCanceled();
    }
};

class ProgressToken
{
    QFutureInterfaceBase *m_future;


    Q_DISABLE_COPY_MOVE(ProgressToken);
public:
    explicit ProgressToken(QFutureInterfaceBase *f) : m_future(f) {}

    void reportProgressRange(int min, int max) const {
        m_future->setProgressRange(min, max);
    }

    void reportProgressValue(int val) const {
        m_future->setProgressValue(val);
    }

    void reportProgressValueAndText(int val, const QString &text) const {
        m_future->setProgressValueAndText(val, text);
    }

    int progressValue(void) const {
        return m_future->progressValue();
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
        ProgressToken progressToken(this);
        this->reportResult(m_func(cancelToken, progressToken));

        this->reportFinished();
    }
};

template <typename TFunctor, typename TResult = typename std::result_of<TFunctor&&(const CancelToken&, const ProgressToken&)>::type>
QFuture<TResult> run(TFunctor &&func)
{
    return (new InternalRunTask<TFunctor, TResult>(std::forward<TFunctor>(func)))->start();
}


template <typename TFunctor>
struct priv_invoker
{
    template<typename... TArgs>
    static void invoke(QObject *, const TFunctor &functor, TArgs &&... args) {
        functor(std::forward<TArgs>(args)...);
    }
};

template <typename TClass, typename... TArgs>
struct priv_invoker<void(TClass::*)(TArgs...)>
{
    static void invoke(TClass *obj, void(TClass::* func)(TArgs...), TArgs &&... args) {
        (obj->*func)(std::forward<TArgs>(args)...);
    }
};

template <typename TClass, typename... TArgs>
struct priv_invoker<void(TClass::* &)(TArgs...)>
{
    static void invoke(TClass *obj, void(TClass::* func)(TArgs...), TArgs &&... args) {
        (obj->*func)(std::forward<TArgs>(args)...);
    }
};

template <typename TClass, typename... TArgs>
struct priv_invoker<void(TClass::* const &)(TArgs...)>
{
    static void invoke(TClass *obj, void(TClass::* func)(TArgs...), TArgs &&... args) {
        (obj->*func)(std::forward<TArgs>(args)...);
    }
};

template <typename TClass, typename... TArgs>
struct priv_invoker<void(TClass::* &&)(TArgs...)>
{
    static void invoke(TClass *obj, void(TClass::* func)(TArgs...), TArgs &&... args) {
        (obj->*func)(std::forward<TArgs>(args)...);
    }
};

template<std::size_t ...>
struct priv_seq { };

template<std::size_t N, std::size_t ...S>
struct priv_gens : priv_gens<N-1, N-1, S...> { };

template<std::size_t ...S>
struct priv_gens<0, S...> {
    typedef priv_seq<S...> type;
};

template<typename... TArgs>
struct priv_tuple_invoker
{
    template<typename TScope, typename TCallable, typename TTuple>
    static void invoke(TScope *scope, TCallable &&callable, TTuple &&tuple)
    {
        invoke_helper(typename priv_gens<sizeof...(TArgs)>::type(), scope, std::forward<TCallable>(callable), std::forward<TTuple>(tuple));
    }


    template<typename TScope, typename TCallable, typename TTuple, std::size_t ...S>
    static void invoke_helper(priv_seq<S...>, TScope *scope, TCallable &&callable, TTuple &&tuple)
    {
        priv_invoker<TCallable>::invoke(scope, std::forward<TCallable>(callable), std::get<S>(std::forward<TTuple>(tuple))...);
    }
};

template <typename TResult, typename TScope, typename TCallable>
void handle_result(const QFuture<TResult> &future, TScope *scope, TCallable &&callable)
{
    QFutureWatcher<TResult> *watcher = new QFutureWatcher<TResult>(scope);
    QObject::connect(watcher, &QFutureWatcherBase::finished, watcher, &QObject::deleteLater);
    QObject::connect(watcher, &QFutureWatcherBase::finished, scope, [=]() {
        if (watcher->isCanceled())
            return;

        priv_invoker<TCallable>::invoke(scope, callable, watcher->result());
    });
    watcher->setFuture(future);
}

template <typename TScope, typename TCallable, typename... TArgs>
void handle_result_tuple_unpack(const QFuture<std::tuple<TArgs...>> &future, TScope *scope, TCallable &&callable)
{
    QFutureWatcher<std::tuple<TArgs...>> *watcher = new QFutureWatcher<std::tuple<TArgs...>>(scope);
    QObject::connect(watcher, &QFutureWatcherBase::finished, watcher, &QObject::deleteLater);
    QObject::connect(watcher, &QFutureWatcherBase::finished, scope, [=]() {
        if (watcher->isCanceled())
            return;

        priv_tuple_invoker<TArgs...>::invoke(scope, callable, watcher->result());
    });
    watcher->setFuture(future);
}

class FutureFinishWaiter {
    QFuture<void> m_future;

public:
    template<typename T>
    FutureFinishWaiter(const QFuture<T> &future) : m_future(QFuture<void>(future)) {}
    ~FutureFinishWaiter() { m_future.waitForFinished(); }
};

} // namespace TaskRunner

#endif // TASKRUNNER_H
