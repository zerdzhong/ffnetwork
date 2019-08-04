#include "thread.h"
#include "log/log_macro.h"
#include "utils/time_utils.h"
#include <stdio.h>

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
    : owned_(true),
      blocking_calls_allowed_(true) {
    SetName("Thread", this);  // default name
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
    }
}

void* Thread::PreRun(void* pv) {
    ThreadInit* init = static_cast<ThreadInit*>(pv);
    ThreadManager::Instance()->SetCurrentThread(init->thread);

    #if defined(LINUX) || defined(ANDROID)
        prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name));
    #elif defined(MAC) || defined(IOS)
        pthread_setname_np(name);
    #endif
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
//TO-DO
}

void Thread::ReceiveSends() {
    //TO-DO
}

bool Thread::WrapCurrent() {
    return WrapCurrentWithThreadManager(ThreadManager::Instance(), true);
}

void Thread::UnwrapCurrent() {
    // Clears the platform-specific thread-specific storage.
    ThreadManager::Instance()->SetCurrentThread(NULL);
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
    thread_manager->SetCurrentThread(this);
    return true;
}

}