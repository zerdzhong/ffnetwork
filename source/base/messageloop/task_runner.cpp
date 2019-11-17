//
// Created by zerdzhong on 2019/10/4.
//

#include "task_runner.h"

#include <utility>
#include "message_loop_impl.h"
#include "message_loop.h"
#include "time/time_point.h"

namespace ffbase {

TaskRunner::TaskRunner(std::shared_ptr<MessageLoopImpl> loop) : loop_(std::move(loop)) {
    
}

TaskRunner::~TaskRunner() = default;

void TaskRunner::PostTask(closure task) {
    loop_->PostTask(std::move(task), TimePoint::Now());
}

void TaskRunner::PostTaskForTime(closure task, TimePoint target_time) {
    loop_->PostTask(std::move(task), target_time);
}

void TaskRunner::PostDelayTask(closure task, TimeDelta delay) {
    loop_->PostTask(std::move(task), TimePoint::Now() + delay);
}

bool TaskRunner::RunTasksOnCurrentThread() {
    if (!MessageLoop::IsInitializedForCurrentThread()) {
        return false;
    }
    
    const auto current_queue_id = MessageLoop::GetCurrentTaskQueueId();
    const auto loop_queue_id = loop_->GetTaskQueueId();

    return current_queue_id == loop_queue_id;

}

void TaskRunner::RunNowOrPostTask(const std::shared_ptr<TaskRunner>& runner, const closure& task) {
    if (runner->RunTasksOnCurrentThread()) {
        task();
    } else {
        runner->PostTask(task);
    }
}

}
