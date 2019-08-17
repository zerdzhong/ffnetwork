//
// Created by zhongzhendong on 2019-08-17.
//

#ifndef FFNETWORK_ASYNC_INVOKER_INL_H
#define FFNETWORK_ASYNC_INVOKER_INL_H

#include "thread.h"

namespace ffnetwork {
    class AsyncInvoker;

    class AsyncClosure {
    public:
        // Runs the asynchronous task, and triggers a callback to the calling
        // thread if needed. Should be called from the target thread.
        virtual void Execute() = 0;
    protected:
        ~AsyncClosure() {}
    };

    template <class FunctorT>
    class FireAndForgetAsyncClosure : public AsyncClosure {
    public:
        explicit FireAndForgetAsyncClosure(const FunctorT& functor)
                : functor_(functor) {}
        virtual void Execute() {
            functor_();
        }
    private:
        FunctorT functor_;
    };

}

#endif //FFNETWORK_ASYNC_INVOKER_INL_H
