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

        void Flush(Thread* thread, uint32_t id = MQID_ANY);

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
}

#endif //FFNETWORK_ASYNC_INVOKER_H
