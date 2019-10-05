//
// Created by zerdzhong on 2019/10/4.
//

#include "message_loop_task_queues.h"

namespace ffbase {

#pragma mark- life_cycle
MessageLoopTaskQueues* MessageLoopTaskQueues::GetInstance() {
    ScopedMutex creation(creation_mutex_);
    
    if (!instance_) {
        instance_ = new MessageLoopTaskQueues();
    }
    
    return instance_;
    
}

TaskQueueId MessageLoopTaskQueues::CreateTaskQueue() {
    UniqueLock lock(*queue_meta_mutex_);
    
    TaskQueueId loop_id = TaskQueueId(task_queue_id_counter_);
    ++task_queue_id_counter_;
    queue_locks_[loop_id] = std::unique_ptr<std::mutex>(new std::mutex());
    
    return loop_id;
}

MessageLoopTaskQueues::MessageLoopTaskQueues() :
task_queue_id_counter_(0),
order_(0)
{
    queue_meta_mutex_ = std::unique_ptr<SharedMutex>(SharedMutex::Create());
}

MessageLoopTaskQueues::~MessageLoopTaskQueues() = default;

void MessageLoopTaskQueues::Dispose(TaskQueueId queue_id) {
    
}

void MessageLoopTaskQueues::DisposeTasks(TaskQueueId queue_id) {
}

#pragma mark- task_method

void MessageLoopTaskQueues::RegisterTask(TaskQueueId queue_id, std::function<void()> task, TimePoint target_time) {
    
}

bool MessageLoopTaskQueues::HasPendingTask(TaskQueueId queue_id) const {
    
}

void MessageLoopTaskQueues::GetTasksToRunNow(TaskQueueId queue_id, FlushType type, std::vector<std::function<void()>> invocations) {
    
}

size_t MessageLoopTaskQueues::GetNumPendingTasks(TaskQueueId queue_id) const {
    
}

#pragma mark- private_method

std::mutex& MessageLoopTaskQueues::GetMutex(TaskQueueId queue_id) const {
    SharedLock reader_lock(*queue_meta_mutex_);
    return *queue_locks_.at(queue_id);
}

}
