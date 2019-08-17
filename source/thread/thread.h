#ifndef FFNETWORK_THREAD_H
#define FFNETWORK_THREAD_H

#include <pthread.h>
#include <string>
#include "message_queue.h"
#include "event.h"

namespace ffnetwork {

    class Thread;

    class ThreadManager {
    public:
        static const int kForever = -1;

        ThreadManager();
        ~ThreadManager();

        static ThreadManager* Instance();

        Thread* CurrentThread();
        void SetCurrentThread(Thread* thread);

        Thread *WrapCurrentThread();
        void UnwrapCurrentThread();
    private:
        pthread_key_t key_{};

        void operator=(const ThreadManager&) = delete;
        ThreadManager(const ThreadManager&) = delete;
    };

    struct _SendMessage {
        _SendMessage() {}
        Thread *thread;
        Message msg;
        bool *ready;
    };

    class Runnable {
    public:
        virtual ~Runnable() {}
        virtual void Run(Thread* thread) = 0;
    protected:
        Runnable() {}
    private:
        void operator=(const Runnable&) = delete;
        Runnable(const Runnable&) = delete;
    };

    class Thread : public MessageQueue {
    public:
        Thread();
        ~Thread() override;

        static Thread* Current();

        bool IsCurrent() const {
            return Current() == this;
        }

        static bool SleepMs(int millis);
        static void AssertBlockingIsAllowedOnCurrentThread();

        const std::string& name() const { return name_; }
        bool SetName(const std::string& name, const void* obj);

        bool Start(Runnable* runnable = NULL);
        void Stop();
        void Run();

        void Send(MessageHandler* phandler,
                    uint32_t id = 0,
                    MessageData* pdata = NULL);


        template <class ReturnT, class FunctorT>
        ReturnT Invoke(const FunctorT& functor) {
            InvokeBegin();
            FunctorMessageHandler<ReturnT, FunctorT> handler(functor);
            Send(&handler);
            InvokeEnd();
            return handler.result();
        }

        // From MessageQueue
        void Clear(MessageHandler* phandler,
                uint32_t id = MQID_ANY,
                MessageList* removed = NULL) override;
        void ReceiveSends() override;

        bool ProcessMessages(int cms);
        bool IsOwned();

        pthread_t GetPThread() {
            return thread_;
        }

        bool SetAllowBlockingCalls(bool allow);

        bool WrapCurrent();
        void UnwrapCurrent();
    protected:
        void SafeWrapCurrent();
        void Join();
    private:
        static void *PreRun(void *pv);
        static void SetThreadName(const char* name);

        bool WrapCurrentWithThreadManager(ThreadManager* thread_manager,
                                    bool need_synchronize_access);

        bool running() { return running_.Wait(0); }

        void ReceiveSendsFromThread(const Thread* source);

        bool PopSendMessageFromThread(const Thread* source, _SendMessage* msg);

        void InvokeBegin();
        void InvokeEnd();
        std::list<_SendMessage> send_list_;
        std::string name_;
        Event running_;  // Signalled means running.

        pthread_t thread_;

        bool owned_;
        bool blocking_calls_allowed_;

        friend class ThreadManager;

        void operator=(const Thread&) = delete;
        Thread(const Thread&) = delete;
    };

    class AutoThread : public Thread {
    public:
        explicit AutoThread();
        ~AutoThread() override;
    private:
        void operator=(const AutoThread&) = delete;
        AutoThread(const AutoThread&) = delete;
    };
}

#endif