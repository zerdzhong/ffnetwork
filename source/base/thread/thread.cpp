//
// Created by zerdzhong on 2019/10/13.
//

#include "thread.h"
#include "waitable_event.h"
#include "messageloop/message_loop.h"
#include "logging.h"
#include <pthread.h>

#if LINUX || ANDROID
#include <sys/prctl.h>
#endif

namespace ffbase {

#pragma mark- life_cycle
Thread::Thread(const std::string& name) :
joined_(false),
name_(name)
{

}

Thread::~Thread() {
    Join();
}

#pragma mark- public_method

void Thread::Start(closure thread_func) {

    AutoResetWaitableEvent ev;
    std::shared_ptr<TaskRunner> task_runner;
    std::string thread_name = name_;

    thread_ = std::unique_ptr<std::thread>(new std::thread([&ev, &task_runner, thread_name, thread_func]{
        SetCurrentThreadName(thread_name);
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        task_runner = loop.GetTaskRunner();
        ev.Signal();

        if (thread_func) {
            thread_func();
        } else {
            loop.Run();
        }
    }));

    ev.Wait();
    task_runner_ = task_runner;
}

void Thread::Join() {
    if (joined_) {
        return;
    }
    
    joined_ = true;
    task_runner_->PostTask([](){
        MessageLoop::GetCurrent().Terminate();
    });
    thread_->join();
}

std::shared_ptr<TaskRunner> Thread::GetTaskRunner() const {
    return task_runner_;
}

std::string Thread::GetName() const {
    return name_;
}

void Thread::SetCurrentThreadName(const std::string& name) {
  if (name.empty()) {
      return;
  }
#if MAC
    pthread_setname_np(name.c_str());
#elif LINUX || ANDROID
    prctl(PR_SET_NAME, reinterpret_cast<unsigned long>(name.c_str()));
#else
    FF_DLOG(INFO) << "Could not set the thread name to '" << name
                  << "' on this platform.";
#endif
}

}
