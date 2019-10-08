//
// Created by zerdzhong on 2019/10/4.
//

#ifndef FFBASE_MESSAGE_LOOP_IMPL_H
#define FFBASE_MESSAGE_LOOP_IMPL_H

#include <memory>
#include <functional>
#include <cstdint>
#include <atomic>
#include "macros.h"
#include "time/time_point.h"
#include "message_loop_task_queues.h"

namespace ffbase {

class MessageLoopImpl : public Wakeable {
public:
    static std::shared_ptr<MessageLoopImpl>Create();
    virtual ~MessageLoopImpl();
    
    virtual void Run() = 0;
    virtual void Terminate() = 0;
    
    void PostTask(std::function<void()> task, TimePoint target_time);
    void AddTaskObserver(intptr_t key, std::function<void()> callback);
    void RemoveTaskObserver(intptr_t key);
    
    
    void DoRun();
    void DoTerminate();
    
    virtual TaskQueueId GetTaskQueueId() const;
    
protected:
    friend class MessageLoop;
    void RunExpiredTasksNow();
    void RunSingleExpiredTaskNow();
    MessageLoopImpl();
    
private:
    std::shared_ptr<MessageLoopTaskQueues> task_queues_;
    TaskQueueId queue_id_;
    std::atomic_bool terminated_;
    
    void FlushTasks(FlushType type);
    
    FF_DISALLOW_COPY_AND_ASSIGN(MessageLoopImpl);
};

}

#endif //FFBASE_MESSAGE_LOOP_IMPL_H
