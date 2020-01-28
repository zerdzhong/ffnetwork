//
// Created by zerdzhong on 2019/10/4.
//

#ifndef FFBASE_MESSAGE_LOOP_TASK_QUEUES_H
#define FFBASE_MESSAGE_LOOP_TASK_QUEUES_H

#include "delayed_task.h"
#include "macros.h"
#include "thread/mutex.h"
#include "time/time_point.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

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

class Wakeable {
public:
  virtual ~Wakeable() {}
  virtual void WakeUp(TimePoint timepoint) = 0;
};

class TaskQueueEntry {
public:
  using TaskObservers = std::map<intptr_t, closure>;
  TaskObservers task_observers;
  DelayedTaskQueue delayed_tasks;
  Wakeable *wakeable;

  TaskQueueEntry();

private:
  FF_DISALLOW_COPY_AND_ASSIGN(TaskQueueEntry);
};

enum class FlushType {
  kSingle,
  kAll,
};

// This class keeps track of all the tasks and observers that
// need to be run on it's MessageLoopImpl. This also wakes up the
// loop at the required times.
class MessageLoopTaskQueues {
public:
  static MessageLoopTaskQueues *GetInstance();
  TaskQueueId CreateTaskQueue();
  void Dispose(TaskQueueId queue_id);
  void DisposeTasks(TaskQueueId queue_id);

  ~MessageLoopTaskQueues();

  // Task
  void RegisterTask(TaskQueueId queue_id, closure task, TimePoint target_time);
  bool HasPendingTask(TaskQueueId queue_id) const;
  void GetTasksToRunNow(TaskQueueId queue_id, FlushType type,
                        std::vector<closure> &invocations);
  size_t GetNumPendingTasks(TaskQueueId queue_id) const;

  // Observer
  void AddTaskObserver(TaskQueueId queue_id, intptr_t key, closure callback);
  void RemoveTaskObserver(TaskQueueId quque_id, intptr_t key);
  std::vector<closure> GetObserversToNotify(TaskQueueId queue_id) const;

  //
  void SetWakeable(TaskQueueId queue_id, Wakeable *wakeable);

private:
  MessageLoopTaskQueues();

  using Mutexes = std::vector<std::unique_ptr<std::mutex>>;

  void WakeUpUnlocked(TaskQueueId queue_id, TimePoint time) const;
  std::mutex &GetMutex(TaskQueueId queue_id) const;

  bool HasPendingTasksUnlocked(TaskQueueId queue_id) const;
  const DelayedTask &PeekNextTaskUnlocked(TaskQueueId owner_id,
                                          TaskQueueId &top_queue_id) const;

  TimePoint GetNextWakeTimeUnlocked(TaskQueueId queue_id) const;

  static std::mutex creation_mutex_;
  static MessageLoopTaskQueues *instance_ FF_GUARDED_BY(creation_mutex_);

  std::unique_ptr<SharedMutex> queue_meta_mutex_;
  std::map<TaskQueueId, std::unique_ptr<TaskQueueEntry>> queue_entries_;
  std::map<TaskQueueId, std::unique_ptr<std::mutex>> queue_locks_;

  size_t task_queue_id_counter_;
  std::atomic_int order_;

  FF_DISALLOW_COPY_ASSIGN_AND_MOVE(MessageLoopTaskQueues);
};

} // namespace ffbase

#endif // FFBASE_MESSAGE_LOOP_TASK_QUEUES_H
