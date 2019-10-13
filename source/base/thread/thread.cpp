//
// Created by zerdzhong on 2019/10/13.
//

#include "thread.h"
#include "waitable_event.h"
#include "messageloop/message_loop.h"
#include "logging.h"
#include <pthread.h>



namespace ffbase {

#pragma mark- life_cycle
Thread::Thread(const std::string& name) :
joined_(false),
name_(name)
{
    AutoResetWaitableEvent ev;
    std::shared_ptr<TaskRunner> task_runner;
    
    thread_ = std::unique_ptr<std::thread>(new std::thread([&ev, &task_runner, name]{
        SetCurrentThreadName(name);
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        task_runner = loop.GetTaskRunner();
        ev.Signal();
        loop.Run();
    }));
    
    ev.Wait();
    task_runner_ = task_runner;
}

Thread::~Thread() {
    Join();
}

#pragma mark- public_method
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
  if (name == "") {
      return;
  }
#if MAC
  pthread_setname_np(name.c_str());
#elif LINUX || ANDROID
  pthread_setname_np(pthread_self(), name.c_str());
#else
  FF_DLOG(INFO) << "Could not set the thread name to '" << name
                 << "' on this platform.";
#endif
}

}
