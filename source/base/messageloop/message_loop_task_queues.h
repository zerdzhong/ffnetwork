//
// Created by zerdzhong on 2019/10/4.
//

#ifndef FFBASE_MESSAGE_LOOP_TASK_QUEUES_H
#define FFBASE_MESSAGE_LOOP_TASK_QUEUES_H

#include "macros.h"
#include "time/time_point.h"
#include "thread/mutex.h"
#include <map>
#include <mutex>
#include <vector>
#include <memory>
#include <utility>
#include <cstdint>
#include <functional>

namespace ffbase {

class TaskQueueId {
public:
    static const size_t kUnmerged;
    explicit TaskQueueId(size_t value) : value_(value) {}
    operator int() const { return value_; }
    
private:
    size_t value_ = kUnmerged;
};

static const TaskQueueId _kUnmerged = TaskQueueId(TaskQueueId::kUnmerged);

enum class FlushType {
  kSingle,
  kAll,
};

// This class keeps track of all the tasks and observers that
// need to be run on it's MessageLoopImpl. This also wakes up the
// loop at the required times.
class MessageLoopTaskQueues {
public:
    static MessageLoopTaskQueues* GetInstance();
    TaskQueueId CreateTaskQueue();
    void Dispose(TaskQueueId queue_id);
    void DisposeTasks(TaskQueueId queue_id);
    
    //Task
    void RegisterTask(TaskQueueId queue_id, std::function<void()> task, TimePoint target_time);
    bool HasPendingTask(TaskQueueId queue_id) const;
    void GetTasksToRunNow(TaskQueueId queue_id, FlushType type, std::vector<std::function<void()>> invocations);
    size_t GetNumPendingTasks(TaskQueueId queue_id) const;
    
    //Observer
    void AddTaskObserver(TaskQueueId queue_id, intptr_t key, std::function<void()> callback);
    void RemoveTaskObserver(TaskQueueId quque_id, intptr_t key);
    std::vector<std::function<void()>>GetObserversToNotify(TaskQueueId queue_id) const;
    
    //
    
private:
    MessageLoopTaskQueues();
    ~MessageLoopTaskQueues();
    
    using Mutexes = std::vector<std::unique_ptr<std::mutex>>;
    
    void WakeUpUnlocked(TaskQueueId queue_id, TimePoint time) const;
    std::mutex& GetMutex(TaskQueueId queue_id) const;
    
    bool HasPendingTasksUnlocked(TaskQueueId queue_id) const;

    TimePoint GetNextWakeTimeUnlocked(TaskQueueId queue_id) const;

    static std::mutex creation_mutex_;
    static MessageLoopTaskQueues* instance_;
    
    std::unique_ptr<SharedMutex> queue_meta_mutex_;
    std::map<TaskQueueId, std::unique_ptr<std::mutex>> queue_locks_;

    size_t task_queue_id_counter_;
    std::atomic_int order_;
    
    FF_DISALLOW_COPY_ASSIGN_AND_MOVE(MessageLoopTaskQueues);
};

}


#endif //FFBASE_MESSAGE_LOOP_TASK_QUEUES_H
