//
// Created by zhongzhendong on 2019-08-17.
//

#ifndef FFNETWORK_ASYNC_INVOKER_INL_H
#define FFNETWORK_ASYNC_INVOKER_INL_H

#include <functional>
#include "thread.h"
#include "callback.h"


namespace ffnetwork {
    class AsyncInvoker;

    class AsyncClosure {
    public:
        // Runs the asynchronous task, and triggers a callback to the calling
        // thread if needed. Should be called from the target thread.
        virtual void Execute() = 0;
    protected:
        ~AsyncClosure() = default;
    };

    template <class FunctorT>
    class FireAndForgetAsyncClosure : public AsyncClosure {
    public:
        explicit FireAndForgetAsyncClosure(const FunctorT& functor)
                : functor_(functor) {}
        void Execute() override {
            functor_();
        }
    private:
        FunctorT functor_;
    };

    class NotifyingAsyncClosureBase : public AsyncClosure,
                                      public sigslot::has_slots<> {
    public:
        ~NotifyingAsyncClosureBase() override;
    protected:
        NotifyingAsyncClosureBase(AsyncInvoker* invoker, Thread* calling_thread);
        void TriggerCallback();
        void SetCallback(const Callback0<void>& callback) {
            CriticalScope cs(&crit_);
            callback_ = callback;
        }
        bool CallbackCanceled() const { return calling_thread_ == NULL; }
    private:
        Callback0<void> callback_;
        CriticalSection crit_;
        AsyncInvoker* invoker_;
        Thread* calling_thread_;
        void CancelCallback();
    };

    // Closures that have a non-void return value and require a callback.
    template <class ReturnT, class FunctorT, class HostT>
    class NotifyingAsyncClosure : public NotifyingAsyncClosureBase {
    public:
        NotifyingAsyncClosure(AsyncInvoker* invoker,
                              Thread* calling_thread,
                              const FunctorT& functor,
                              void (HostT::*callback)(ReturnT),
                              HostT* callback_host)
                :  NotifyingAsyncClosureBase(invoker, calling_thread),
                   functor_(functor),
                   callback_(callback),
                   callback_host_(callback_host) {}
        virtual void Execute() {
            ReturnT result = functor_();
            if (!CallbackCanceled()) {
                SetCallback(Callback0<void>(std::bind(callback_, callback_host_, result)));
                TriggerCallback();
            }
        }
    private:
        FunctorT functor_;
        void (HostT::*callback_)(ReturnT);
        HostT* callback_host_;
    };

    // Closures that have a void return value and require a callback.
    template <class FunctorT, class HostT>
    class NotifyingAsyncClosure<void, FunctorT, HostT>
            : public NotifyingAsyncClosureBase {
    public:
        NotifyingAsyncClosure(AsyncInvoker* invoker,
                              Thread* calling_thread,
                              const FunctorT& functor,
                              void (HostT::*callback)(),
                              HostT* callback_host)
                : NotifyingAsyncClosureBase(invoker, calling_thread),
                  functor_(functor) {
            SetCallback(sigslot::signal0<void>(std::bind(callback, callback_host)));
        }
        virtual void Execute() {
            functor_();
            TriggerCallback();
        }
    private:
        FunctorT functor_;
    };

}

#endif //FFNETWORK_ASYNC_INVOKER_INL_H
