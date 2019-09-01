#include "thread.h"
#include "log/log_macro.h"
#include "utils/time_utils.h"
#include <cstdio>
#include <cassert>

#if defined(LINUX)
#include <sys/prctl.h>
#include <sys/syscall.h>
#endif

namespace ffnetwork {

#pragma mark- ThreadManager

ThreadManager* ThreadManager::Instance() {
    static ThreadManager& thread_manager = *new ThreadManager();
    return &thread_manager;
}

Thread* Thread::Current() {
    return ThreadManager::Instance()->CurrentThread();
}

ThreadManager::ThreadManager() {
    pthread_key_create(&key_, NULL);
    WrapCurrentThread();
}

ThreadManager::~ThreadManager() {
    UnwrapCurrentThread();
    pthread_key_delete(key_);
}

Thread *ThreadManager::CurrentThread() {
    return static_cast<Thread *>(pthread_getspecific(key_));
}
void ThreadManager::SetCurrentThread(Thread *thread) {
    pthread_setspecific(key_, thread);
}

Thread *ThreadManager::WrapCurrentThread() {
    Thread* result = CurrentThread();
    if (NULL == result) {
        result = new Thread();
        result->WrapCurrentWithThreadManager(this, true);
    }
    return result;
}
void ThreadManager::UnwrapCurrentThread() {
    Thread* t = CurrentThread();
    if (t && !(t->IsOwned())) {
        t->UnwrapCurrent();
        delete t;
    }
}

struct ThreadInit {
    Thread* thread;
    Runnable* runnable;
};

#pragma mark- Thread

Thread::Thread()
    : running_(true, false)
      owned_(true),
      blocking_calls_allowed_(true),
{
    SetName("ffnetwork-thread", this);  // default name
}

Thread::~Thread() {
    Stop();
    Clear(NULL);
}

bool Thread::SetName(const std::string& name, const void* obj) {
    if (running()) return false;
    name_ = name;
    if (obj) {
        char buf[16];
        sprintf(buf, " 0x%p", obj);
        name_ += buf;
    }

    return true;
}

bool Thread::Start(Runnable* runnable) {
    if (!owned_) return false;
    if (running()) return false;

    Restart();  // reset fStop_ if the thread is being restarted
    // Make sure that ThreadManager is created on the main thread before
    // we start a new thread.

    ThreadManager::Instance();
    ThreadInit* init = new ThreadInit;
    init->thread = this;
    init->runnable = runnable;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int error_code = pthread_create(&thread_, &attr, PreRun, init);
    if (0 != error_code) {
        LOGD("Unable to create pthread, error %d", error_code);
        return false;
    }

    running_.Set();
    
    return true;
}

void Thread::Stop() {
    MessageQueue::Quit();
    Join();
}

void Thread::Join() {
    if (running()) {
        if (Current() && !Current()->blocking_calls_allowed_) {
            LOGD("Waiting for the thread to join, but blocking calls have been disallowed");
        }

        void *pv;
        pthread_join(thread_, &pv);

        running_.Reset();
    }
}

void* Thread::PreRun(void* pv) {
    ThreadInit* init = static_cast<ThreadInit*>(pv);
    ThreadManager::Instance()->SetCurrentThread(init->thread);
    Thread::SetThreadName(init->thread->name_.c_str());
    {
        if (init->runnable) {
            init->runnable->Run(init->thread);
        } else {
            init->thread->Run();
        }

        delete init;
        return NULL;
    }
}

void Thread::Run() {
    ProcessMessages(kForever);
}

bool Thread::ProcessMessages(int cmsLoop) {
    uint32_t msEnd = (kForever == cmsLoop) ? 0 : TimeAfter(cmsLoop);
    int cmsNext = cmsLoop;
    while (true) {
        Message msg;
        if (!Get(&msg, cmsNext))
            return !IsQuitting();
        Dispatch(&msg);
        if (cmsLoop != kForever) {
            cmsNext = TimeUntil(msEnd);
            if (cmsNext < 0)
            return true;
        }
    }
}

bool Thread::IsOwned() {
    return owned_;
}


void Thread::Clear(MessageHandler* phandler,
                   uint32_t id,
                   MessageList* removed) {
    CriticalScope cs(&critical_section_);

    auto iter = send_list_.begin();

    while (iter != send_list_.end()) {
        _SendMessage message = *iter;
        if (message.msg.Match(phandler, id)) {
            if (removed) {
                removed->push_back(message.msg);
            } else {
                delete message.msg.pdata;
            }

            iter = send_list_.erase(iter);
            *message.ready = true;
            continue;
        }

        ++iter;
    }

    MessageQueue::Clear(phandler, id, removed);

}

void Thread::ReceiveSends() {
    ReceiveSendsFromThread(NULL);
}

bool Thread::WrapCurrent() {
    return WrapCurrentWithThreadManager(ThreadManager::Instance(), true);
}

void Thread::UnwrapCurrent() {
    // Clears the platform-specific thread-specific storage.
    ThreadManager::Instance()->SetCurrentThread(NULL);
    running_.Reset();
}

void Thread::SafeWrapCurrent() {
    WrapCurrentWithThreadManager(ThreadManager::Instance(), false);
}

bool Thread::WrapCurrentWithThreadManager(ThreadManager* thread_manager,
                                          bool need_synchronize_access) {
    if (running()) {
        return false;
    }

    thread_ = pthread_self();
    
    owned_ = false;
    running_.Set();
    thread_manager->SetCurrentThread(this);
    return true;
}

    void Thread::Send(MessageHandler *phandler, uint32_t id, MessageData *pdata) {
        if (fStop_) {
            return;
        }

        Message msg;
        msg.phandler = phandler;
        msg.message_id = id;
        msg.pdata = pdata;
        if (IsCurrent()) {
            phandler->OnMessage(&msg);
            return;
        }


        AutoThread thread;
        Thread *current_thread = Thread::Current();
        assert(current_thread != NULL);  // AutoThread ensures this

        bool ready = false;
        {
            CriticalScope cs(&critical_section_);
            _SendMessage send_msg;
            send_msg.thread = current_thread;
            send_msg.msg = msg;
            send_msg.ready = &ready;
            send_list_.push_back(send_msg);
        }

        critical_section_.Enter();
        while (!ready) {
            critical_section_.Leave();
            // We need to limit "ReceiveSends" to |this| thread to avoid an arbitrary
            // thread invoking calls on the current thread.
            current_thread->ReceiveSendsFromThread(this);
            critical_section_.Enter();
        }
        critical_section_.Leave();

    }

    void Thread::ReceiveSendsFromThread(const Thread *source) {
        _SendMessage send_msg;

        while(PopSendMessageFromThread(source, &send_msg)) {
            send_msg.msg.phandler->OnMessage(&send_msg.msg);
            *send_msg.ready = true;
        }

    }

    bool Thread::PopSendMessageFromThread(const Thread *source, _SendMessage *msg) {
        CriticalScope sc(&critical_section_);

        for (auto it = send_list_.begin();
             it != send_list_.end(); ++it) {
            if (it->thread == source || source == NULL) {
                *msg = *it;
                send_list_.erase(it);
                return true;
            }
        }

        return false;
    }

    bool Thread::SleepMs(int millis) {
        return false;
    }

    // static
    void Thread::AssertBlockingIsAllowedOnCurrentThread() {
#if defined(DEBUG)
        Thread* current = Thread::Current();
        assert(!current || current->blocking_calls_allowed_);
#endif
    }

    bool Thread::SetAllowBlockingCalls(bool allow) {
        return blocking_calls_allowed_ = allow;
    }

    void Thread::InvokeBegin() {

    }

    void Thread::InvokeEnd() {

    }

    void Thread::SetThreadName(const char* name) {
#if defined(WIN)
            struct {
    DWORD dwType;
    LPCSTR szName;
    DWORD dwThreadID;
    DWORD dwFlags;
  } threadname_info = {0x1000, name, static_cast<DWORD>(-1), 0};

  __try {
    ::RaiseException(0x406D1388, 0, sizeof(threadname_info) / sizeof(DWORD),
                     reinterpret_cast<ULONG_PTR*>(&threadname_info));
  } __except (EXCEPTION_EXECUTE_HANDLER) {  // NOLINT
  }
#elif defined(LINUX) || defined(ANDROID)
        prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name));  // NOLINT
#elif defined(MAC) || defined(IOS)
        pthread_setname_np(name);
#endif
    }

    AutoThread::AutoThread() {
        if (!ThreadManager::Instance()->CurrentThread()) {
            ThreadManager::Instance()->SetCurrentThread(this);
        }
    }

    AutoThread::~AutoThread() {
        Stop();
        if (ThreadManager::Instance()->CurrentThread() == this) {
            ThreadManager::Instance()->SetCurrentThread(NULL);
        }
    }
}
