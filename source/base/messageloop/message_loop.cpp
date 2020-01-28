//
// Created by zerdzhong on 2019/10/4.
//

#include "message_loop.h"

#include "logging.h"
#include "message_loop_impl.h"
#include "message_loop_task_queues.h"
#include "task_runner.h"
#include "thread/thread_local.h"
#include <utility>

namespace ffbase {

FF_THREAD_LOCAL ThreadLocalUniquePtr<MessageLoop> tls_message_loop;

MessageLoop &MessageLoop::GetCurrent() {
  auto *loop = tls_message_loop.get();
  FF_CHECK(loop != nullptr)
      << "MessageLoop::EnsureInitializedForCurrentThread was not called on "
         "this thread prior to message loop use.";

  return *loop;
}

void MessageLoop::EnsureInitializedForCurrentThread() {
  if (tls_message_loop.get() != nullptr) {
    return;
  }

  tls_message_loop.reset(new MessageLoop());
}

bool MessageLoop::IsInitializedForCurrentThread() {
  return tls_message_loop.get() != nullptr;
}

TaskQueueId MessageLoop::GetCurrentTaskQueueId() {
  auto *loop = tls_message_loop.get();
  FF_CHECK(loop != nullptr)
      << "MessageLoop::EnsureInitializedForCurrentThread was not called on "
         "this thread prior to message loop use.";
  return loop->GetLoopImpl()->GetTaskQueueId();
}

MessageLoop::MessageLoop()
    : loop_(MessageLoopImpl::Create()),
      task_runner_(std::make_shared<TaskRunner>(loop_)) {
  FF_CHECK(loop_);
  FF_CHECK(task_runner_);
}

MessageLoop::~MessageLoop() = default;

void MessageLoop::Run() { loop_->DoRun(); }

void MessageLoop::RunForTime(TimeDelta delay) { loop_->DoRunForTime(delay); }

bool MessageLoop::IsRunning() { return true; }

void MessageLoop::Terminate() { loop_->DoTerminate(); }

void MessageLoop::AddTaskObserver(intptr_t key,
                                  std::function<void()> callback) {
  loop_->AddTaskObserver(key, std::move(callback));
}

void MessageLoop::RemoveObserver(intptr_t key) {
  loop_->RemoveTaskObserver(key);
}

void MessageLoop::RunExpiredTasksNow() { loop_->RunExpiredTasksNow(); }

std::shared_ptr<TaskRunner> MessageLoop::GetTaskRunner() const {
  return task_runner_;
}

std::shared_ptr<MessageLoopImpl> MessageLoop::GetLoopImpl() const {
  return loop_;
}

} // namespace ffbase
