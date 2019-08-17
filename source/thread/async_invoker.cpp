//
// Created by zhongzhendong on 2019-08-17.
//

#include "async_invoker.h"
#include "log/log_macro.h"

namespace ffnetwork {

    AsyncInvoker::AsyncInvoker() : destroying_(false) {}
    AsyncInvoker::~AsyncInvoker() {
        destroying_ = true;
        // Messages for this need to be cleared *before* our destructor is complete.
        MessageQueueManager::Clear(this);
    }

    void AsyncInvoker::Flush(Thread *thread, uint32_t id) {
        if (destroying_) return;
        // Run this on |thread| to reduce the number of context switches.
        if (Thread::Current() != thread) {
            thread->Invoke<void>(std::bind(&AsyncInvoker::Flush, this, thread, id));
            return;
        }
        MessageList removed;
        thread->Clear(this, id, &removed);
        for (MessageList::iterator it = removed.begin(); it != removed.end(); ++it) {
            // This message was pending on this thread, so run it now.
            thread->Send(it->phandler,
                         it->message_id,
                         it->pdata);
        }
    }

    void AsyncInvoker::OnMessage(ffnetwork::Message *msg) {
        // Get the AsyncClosure shared ptr from this message's data.
        SharedMessageData<AsyncClosure>* data =
                static_cast<SharedMessageData<AsyncClosure>*>(msg->pdata);
        std::shared_ptr<AsyncClosure> closure = data->data();
        delete msg->pdata;
        msg->pdata = NULL;
        // Execute the closure and trigger the return message if needed.
        closure->Execute();
    }

    void AsyncInvoker::DoInvoke(Thread *thread, const std::shared_ptr<AsyncClosure> &closure, uint32_t id) {
        if (destroying_) {
            LOGD("Tried to invoke while destroying the invoker.");
            return;
        }
        thread->Post(this, id, new SharedMessageData<AsyncClosure>(closure));
    }

    void AsyncInvoker::DoInvokeDelayed(Thread *thread, const std::shared_ptr<AsyncClosure> &closure, uint32_t delay_ms,
                                       uint32_t id) {
        if (destroying_) {
            LOGD("Tried to invoke while destroying the invoker.");
            return;
        }
//        thread->PostDelayed(delay_ms,this, id, new SharedMessageData<AsyncClosure>(closure));
    }


}
