//
// Created by zerdzhong on 2019/10/4.
//

#include "message_loop_task_queues.h"
#include "logging.h"
#include <climits>

namespace ffbase {

#pragma mark - global_var

const size_t TaskQueueId::kUnmerged = ULONG_MAX;
std::mutex MessageLoopTaskQueues::creation_mutex_;
MessageLoopTaskQueues *MessageLoopTaskQueues::instance_ = nullptr;

#pragma mark - TaskQueueEntry

TaskQueueEntry::TaskQueueEntry() {
  wakeable = nullptr;
  task_observers = TaskObservers();
  delayed_tasks = DelayedTaskQueue();
}

#pragma mark - life_cycle

MessageLoopTaskQueues *MessageLoopTaskQueues::GetInstance() {
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
  queue_entries_[loop_id] =
      std::unique_ptr<TaskQueueEntry>(new TaskQueueEntry());

  return loop_id;
}

MessageLoopTaskQueues::MessageLoopTaskQueues()
    : queue_meta_mutex_(std::unique_ptr<SharedMutex>(SharedMutex::Create())),
      task_queue_id_counter_(0), order_(0) {}

MessageLoopTaskQueues::~MessageLoopTaskQueues() = default;

void MessageLoopTaskQueues::Dispose(TaskQueueId queue_id) {
  ScopedMutex queue_lock(GetMutex(queue_id));
  queue_entries_.erase(queue_id);
}

void MessageLoopTaskQueues::DisposeTasks(TaskQueueId queue_id) {
  ScopedMutex queue_lock(GetMutex(queue_id));
  const auto &queue = queue_entries_.at(queue_id);
  queue->delayed_tasks = {};
}

#pragma mark - task_method

void MessageLoopTaskQueues::RegisterTask(TaskQueueId queue_id,
                                         std::function<void()> task,
                                         TimePoint target_time) {
  ScopedMutex queue_lock(GetMutex(queue_id));

  auto order = order_++;
  const auto &queue_entry = queue_entries_[queue_id];

  queue_entry->delayed_tasks.push(
      {static_cast<size_t>(order), std::move(task), target_time});

  TaskQueueId loop_to_wake = queue_id;

  auto wakeup_delay_seconds =
      (queue_entry->delayed_tasks.top().GetTargetTime() - TimePoint::Now())
          .ToMilliseconds();

  FF_LOG(INFO) << "RegisterTask now "
               << TimePoint::Now().ToEpochDelta().ToMilliseconds() << " after "
               << wakeup_delay_seconds << " ms";

  WakeUpUnlocked(loop_to_wake,
                 queue_entry->delayed_tasks.top().GetTargetTime());
}

bool MessageLoopTaskQueues::HasPendingTask(TaskQueueId queue_id) const {
  ScopedMutex queue_lock(GetMutex(queue_id));

  return HasPendingTasksUnlocked(queue_id);
}

void MessageLoopTaskQueues::GetTasksToRunNow(
    TaskQueueId queue_id, FlushType type,
    std::vector<std::function<void()>> &invocations) {
  ScopedMutex queue_lock(GetMutex(queue_id));

  if (!HasPendingTasksUnlocked(queue_id)) {
    return;
  }

  const auto now = TimePoint::Now();

  while (HasPendingTasksUnlocked(queue_id)) {
    TaskQueueId top_queue_id = _kUnmerged;
    const auto &top_task = PeekNextTaskUnlocked(queue_id, top_queue_id);

    if (top_task.GetTargetTime() > now) {
      break;
    }

    invocations.emplace_back(top_task.GetTask());
    queue_entries_[top_queue_id]->delayed_tasks.pop();
    if (FlushType::kSingle == type) {
      break;
    }

    if (!HasPendingTasksUnlocked(queue_id)) {
      WakeUpUnlocked(queue_id, TimePoint::Max());
    } else {
      WakeUpUnlocked(queue_id, GetNextWakeTimeUnlocked(queue_id));
    }
  }
}

size_t MessageLoopTaskQueues::GetNumPendingTasks(TaskQueueId queue_id) const {
  ScopedMutex queue_lock(GetMutex(queue_id));

  const auto &queue_entry = queue_entries_.at(queue_id);
  size_t total_task = 0;

  total_task += queue_entry->delayed_tasks.size();

  return total_task;
}

#pragma mark - observer

void MessageLoopTaskQueues::AddTaskObserver(TaskQueueId queue_id, intptr_t key,
                                            std::function<void()> callback) {
  ScopedMutex queue_lock(GetMutex(queue_id));

  FF_DCHECK(callback != nullptr) << "Observer callback must be non-null.";
  queue_entries_[queue_id]->task_observers[key] = std::move(callback);
}

void MessageLoopTaskQueues::RemoveTaskObserver(TaskQueueId queue_id,
                                               intptr_t key) {
  ScopedMutex queue_lock(GetMutex(queue_id));

  queue_entries_[queue_id]->task_observers.erase(key);
}

std::vector<closure>
MessageLoopTaskQueues::GetObserversToNotify(TaskQueueId queue_id) const {
  ScopedMutex queue_lock(GetMutex(queue_id));
  std::vector<closure> observers;

  for (const auto &observer : queue_entries_.at(queue_id)->task_observers) {
    observers.push_back(observer.second);
  }

  return observers;
}

void MessageLoopTaskQueues::SetWakeable(TaskQueueId queue_id,
                                        Wakeable *wakeable) {
  ScopedMutex queue_lock(GetMutex(queue_id));

  FF_CHECK(!queue_entries_[queue_id]->wakeable)
      << "Wakeable can only be set once.";
  queue_entries_.at(queue_id)->wakeable = wakeable;
}

#pragma mark - private_method

std::mutex &MessageLoopTaskQueues::GetMutex(TaskQueueId queue_id) const {
  SharedLock reader_lock(*queue_meta_mutex_);
  return *queue_locks_.at(queue_id);
}

void MessageLoopTaskQueues::WakeUpUnlocked(TaskQueueId queue_id,
                                           TimePoint time) const {
  if (queue_entries_.at(queue_id)->wakeable) {
    queue_entries_.at(queue_id)->wakeable->WakeUp(time);
  }
}

bool MessageLoopTaskQueues::HasPendingTasksUnlocked(
    TaskQueueId queue_id) const {
  const auto &queue_entry = queue_entries_.at(queue_id);

  return !queue_entry->delayed_tasks.empty();
}

TimePoint
MessageLoopTaskQueues::GetNextWakeTimeUnlocked(TaskQueueId queue_id) const {
  TaskQueueId tmp = _kUnmerged;
  return PeekNextTaskUnlocked(queue_id, tmp).GetTargetTime();
}

const DelayedTask &
MessageLoopTaskQueues::PeekNextTaskUnlocked(TaskQueueId owner_id,
                                            TaskQueueId &top_queue_id) const {
  const auto &owner_entry = queue_entries_.at(owner_id);
  const auto &owner_tasks_queue = owner_entry->delayed_tasks;

  const bool have_task = !owner_tasks_queue.empty();

  if (have_task) {
    top_queue_id = owner_id;
  }

  return queue_entries_.at(top_queue_id)->delayed_tasks.top();
}

} // namespace ffbase
