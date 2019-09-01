//
// Created by zhongzhendong on 2019-08-17.
//

#ifndef FFNETWORK_ASYNC_INVOKER_H
#define FFNETWORK_ASYNC_INVOKER_H

#include "thread.h"
#include "async_invoker_inl.h"
#include "construct_macro.h"
#include <memory>

namespace ffnetwork {
    class AsyncInvoker : public MessageHandler {
    public:
        AsyncInvoker();
        ~AsyncInvoker() override ;

        // Call |functor| asynchronously on |thread|, with no callback upon
        // completion. Returns immediately.
        template <class ReturnT, class FunctorT>
        void AsyncInvoke(Thread* thread, const FunctorT& functor, uint32_t id = 0) {
            std::shared_ptr<AsyncClosure> closure(
                    new FireAndForgetAsyncClosure<FunctorT>(functor));
            DoInvoke(thread, closure, id);
        }

        // Call |functor| asynchronously on |thread| with |delay_ms|, with no callback
        // upon completion. Returns immediately.
        template <class ReturnT, class FunctorT>
        void AsyncInvokeDelayed(Thread* thread,
                                const FunctorT& functor,
                                uint32_t delay_ms,
                                uint32_t id = 0) {
            std::shared_ptr<AsyncClosure> closure(
                    new FireAndForgetAsyncClosure<FunctorT>(functor));
            DoInvokeDelayed(thread, closure, delay_ms, id);
        }

        // Call |functor| asynchronously on |thread|, calling |callback| when done.
        template <class ReturnT, class FunctorT, class HostT>
        void AsyncInvoke(Thread* thread,
                         const FunctorT& functor,
                         void (HostT::*callback)(ReturnT),
                         HostT* callback_host,
                         uint32_t id = 0) {
            std::shared_ptr<AsyncClosure> closure(
                    new NotifyingAsyncClosure<ReturnT, FunctorT, HostT> (
                            this, Thread::Current(), functor, callback, callback_host));
            DoInvoke(thread, closure, id);
        }


        void Flush(Thread* thread, uint32_t id = MQID_ANY);

        // Signaled when this object is destructed.
        sigslot::signal0<> SignalInvokerDestroyed;

    private:
        void OnMessage(Message* msg) override;
        void DoInvoke(Thread* thread,
                      const std::shared_ptr<AsyncClosure>& closure,
                      uint32_t id);
        void DoInvokeDelayed(Thread* thread,
                             const std::shared_ptr<AsyncClosure>& closure,
                             uint32_t delay_ms,
                             uint32_t id);
        bool destroying_;
        DISALLOW_COPY_AND_ASSIGN(AsyncInvoker);
    };

    class GuardedAsyncInvoker : public sigslot::has_slots<> {
    public:
        GuardedAsyncInvoker();
        ~GuardedAsyncInvoker() override;
        // Synchronously execute all outstanding calls we own, and wait for calls to
        // complete before returning. Optionally filter by message id. The destructor
        // will not wait for outstanding calls, so if that behavior is desired, call
        // Flush() first. Returns false if the thread has died.
        bool Flush(uint32_t id = MQID_ANY);
        // Call |functor| asynchronously with no callback upon completion. Returns
        // immediately. Returns false if the thread has died.
        template <class ReturnT, class FunctorT>
        bool AsyncInvoke(const FunctorT& functor, uint32_t id = 0) {
            CriticalScope cs(&crit_);
            if (thread_ == nullptr)
                return false;
            invoker_.AsyncInvoke<ReturnT, FunctorT>(thread_, functor, id);
            return true;
        }
        // Call |functor| asynchronously with |delay_ms|, with no callback upon
        // completion. Returns immediately. Returns false if the thread has died.
        template <class ReturnT, class FunctorT>
        bool AsyncInvokeDelayed(const FunctorT& functor,
                                uint32_t delay_ms,
                                uint32_t id = 0) {
            CriticalScope cs(&crit_);
            if (thread_ == nullptr)
                return false;
            invoker_.AsyncInvokeDelayed<ReturnT, FunctorT>(thread_, functor, delay_ms,
                                                           id);
            return true;
        }
        // Call |functor| asynchronously, calling |callback| when done. Returns false
        // if the thread has died.
        template <class ReturnT, class FunctorT, class HostT>
        bool AsyncInvoke(const FunctorT& functor,
                         void (HostT::*callback)(ReturnT),
                         HostT* callback_host,
                         uint32_t id = 0) {
            CriticalScope cs(&crit_);
            if (thread_ == nullptr)
                return false;
            invoker_.AsyncInvoke<ReturnT, FunctorT, HostT>(thread_, functor, callback,
                                                           callback_host, id);
            return true;
        }
        // Call |functor| asynchronously calling |callback| when done. Overloaded for
        // void return. Returns false if the thread has died.
        template <class ReturnT, class FunctorT, class HostT>
        bool AsyncInvoke(const FunctorT& functor,
                         void (HostT::*callback)(),
                         HostT* callback_host,
                         uint32_t id = 0) {
            CriticalScope cs(&crit_);
            if (thread_ == nullptr)
                return false;
            invoker_.AsyncInvoke<ReturnT, FunctorT, HostT>(thread_, functor, callback,
                                                           callback_host, id);
            return true;
        }
    private:
        // Callback when |thread_| is destroyed.
        void ThreadDestroyed();
        CriticalSection crit_;
        Thread* thread_ GUARDED_BY(crit_);
        AsyncInvoker invoker_ GUARDED_BY(crit_);
    };
}

#endif //FFNETWORK_ASYNC_INVOKER_H
