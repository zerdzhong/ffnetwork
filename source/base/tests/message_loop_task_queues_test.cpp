//
// Created by zerdzhong on 2019/10/7.
//

#include "base/messageloop/message_loop_task_queues.h"
#include "gtest/gtest.h"

using namespace ffbase;

class TestWakeable : public Wakeable {
public:
  using WakeUpCall = std::function<void(const TimePoint)>;

  TestWakeable(WakeUpCall call) : wake_up_call_(call) {}

  void WakeUp(TimePoint time_point) override { wake_up_call_(time_point); }

private:
  WakeUpCall wake_up_call_;
};

TEST(MessageLoopTaskQueues, StartWithNoPendingTask) {
  auto task_queues = MessageLoopTaskQueues::GetInstance();
  auto queue_id = task_queues->CreateTaskQueue();
  ASSERT_FALSE(task_queues->HasPendingTask(queue_id));
}

TEST(MessageLoopTaskQueues, RegisterTask) {
  const auto time = TimePoint::Max();
  auto task_queues = MessageLoopTaskQueues::GetInstance();
  auto queue_id = task_queues->CreateTaskQueue();

  task_queues->SetWakeable(queue_id,
                           new TestWakeable([&time](TimePoint wake_time) {
                             ASSERT_TRUE(time == wake_time);
                           }));

  task_queues->RegisterTask(
      queue_id, [] {}, time);
  ASSERT_TRUE(task_queues->HasPendingTask(queue_id));
  ASSERT_TRUE(task_queues->GetNumPendingTasks(queue_id) == 1);
}

TEST(MessageLoopTaskQueues, RegisterTwoTasksAndCount) {
  auto task_queues = MessageLoopTaskQueues::GetInstance();
  auto queue_id = task_queues->CreateTaskQueue();

  task_queues->RegisterTask(
      queue_id, [] {}, TimePoint::Now());
  task_queues->RegisterTask(
      queue_id, [] {}, TimePoint::Max());

  ASSERT_TRUE(task_queues->HasPendingTask(queue_id));
  ASSERT_TRUE(task_queues->GetNumPendingTasks(queue_id) == 2);
}

void TestNotifyObservers(TaskQueueId queue_id) {
  auto task_queue = MessageLoopTaskQueues::GetInstance();
  std::vector<closure> observers = task_queue->GetObserversToNotify(queue_id);
  for (const auto &observer : observers) {
    observer();
  }
}

TEST(MessageLoopTaskQueues, AddRemoveNotifyObserver) {
  auto task_queues = MessageLoopTaskQueues::GetInstance();
  auto queue_id = task_queues->CreateTaskQueue();

  int test = 0;
  intptr_t key = 1;

  task_queues->AddTaskObserver(queue_id, key, [&test]() { test = 1; });
  TestNotifyObservers(queue_id);
  ASSERT_TRUE(test == 1);

  test = 0;
  task_queues->RemoveTaskObserver(queue_id, key);
  TestNotifyObservers(queue_id);
  ASSERT_TRUE(test == 0);
}

TEST(MessageLoopTaskQueues, WakeUpIndependentOfTime) {
  auto task_queues = MessageLoopTaskQueues::GetInstance();
  auto queue_id = task_queues->CreateTaskQueue();

  int num_wakes = 0;

  task_queues->SetWakeable(
      queue_id,
      new TestWakeable([&num_wakes](TimePoint wake_time) { ++num_wakes; }));

  task_queues->RegisterTask(
      queue_id, []() {}, TimePoint::Now());
  task_queues->RegisterTask(
      queue_id, []() {}, TimePoint::Max());

  EXPECT_EQ(num_wakes, 2);
}
