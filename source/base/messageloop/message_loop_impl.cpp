//
// Created by zerdzhong on 2019/10/4.
//

#include "message_loop_impl.h"
#include "logging.h"
#include "message_loop.h"

#ifdef MAC
#include "message_loop_impl_darwin.h"
#endif

namespace ffbase {

std::shared_ptr<MessageLoopImpl> MessageLoopImpl::Create() {
#ifdef MAC
    return std::make_shared<MessageLoopDarwin>();
#endif
    return nullptr;
}

#pragma mark- lifecycle

MessageLoopImpl::MessageLoopImpl() :
task_queues_(MessageLoopTaskQueues::GetInstance()),
queue_id_(task_queues_->CreateTaskQueue()),
terminated_(false)
{
    
}

MessageLoopImpl::~MessageLoopImpl() {
    
}

#pragma mark- task_method
void MessageLoopImpl::PostTask(std::function<void()> task, TimePoint target_time) {
    FF_DCHECK(task != nullptr);
    
    if (terminated_) {
        return;
    }
    
    task_queues_->RegisterTask(queue_id_, task, target_time);
    
}

void MessageLoopImpl::AddTaskObserver(intptr_t key, std::function<void()> callback) {
    FF_DCHECK(callback != nullptr);
    FF_DCHECK(MessageLoop::GetCurrent().GetLoopImpl().get() == this)
    << "Message loop task observer must be added on the same thread as the loop.";
    
    
    if (callback != nullptr) {
        task_queues_->AddTaskObserver(queue_id_, key, callback);
    } else {
        FF_LOG(ERROR) << "Tried to add a null TaskObserver.";
    }
}

void MessageLoopImpl::RemoveTaskObserver(intptr_t key) {
    FF_DCHECK(MessageLoop::GetCurrent().GetLoopImpl().get() == this)
        << "Message loop task observer must be removed from the same thread as "
           "the loop.";
    task_queues_->RemoveTaskObserver(queue_id_, key);
}

void MessageLoopImpl::DoRun() {
    if (terminated_) {
        return;
    }
    
    Run();
    
    terminated_ = true;
    
    RunExpiredTasksNow();
    task_queues_->DisposeTasks(queue_id_);
}

void MessageLoopImpl::DoTerminate() {
    terminated_ = true;
    Terminate();
}

void MessageLoopImpl::FlushTasks(FlushType type) {
    std::vector<closure> invocations;
    
    task_queues_->GetTasksToRunNow(queue_id_, type, invocations);
    
    for (const auto& invocation : invocations) {
        invocation();
        auto observers = task_queues_->GetObserversToNotify(queue_id_);
        
        for (const auto& observer: observers) {
            observer();
        }
    }
}

void MessageLoopImpl::RunExpiredTasksNow() {
    FlushTasks(FlushType::kAll);
}

void MessageLoopImpl::RunSingleExpiredTaskNow() {
    FlushTasks(FlushType::kSingle);
}

TaskQueueId MessageLoopImpl::GetTaskQueueId() const {
    return queue_id_;
}

}
