//
// Created by zerdzhong on 2019/10/4.
//

#include "task_runner.h"
#include "message_loop_impl.h"
#include "message_loop.h"
#include "time/time_point.h"

namespace ffbase {

TaskRunner::TaskRunner(std::shared_ptr<MessageLoopImpl> loop) : loop_(loop) {
    
}

TaskRunner::~TaskRunner() = default;

void TaskRunner::PostTask(closure task) {
    loop_->PostTask(task, TimePoint::Now());
}

void TaskRunner::PostTaskForTime(closure task, TimePoint target_time) {
    loop_->PostTask(task, target_time);
}

void TaskRunner::PostDelayTask(closure task, TimeDelta delay) {
    loop_->PostTask(task, TimePoint::Now() + delay);
}

bool TaskRunner::RunTasksOnCurrentThread() {
    if (!MessageLoop::IsInitializedForCurrentThread()) {
        return false;
    }
    
    const auto current_queue_id = MessageLoop::GetCurrentTaskQueueId();
    const auto loop_queue_id = loop_->GetTaskQueueId();
    
    if (current_queue_id == loop_queue_id) {
        return true;
    }
    
    return false;
}

void TaskRunner::RunNowOrPostTask(std::shared_ptr<TaskRunner> runner, closure task) {
    if (runner->RunTasksOnCurrentThread()) {
        task();
    } else {
        runner->PostTask(task);
    }
}

}
