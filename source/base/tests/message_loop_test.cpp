//
// Created by zerdzhong on 2019/10/7.
//

#include "gtest/gtest.h"
#include "base/messageloop/message_loop.h"
#include "time/time_point.h"
#include "logging.h"
#include <thread>
#include <condition_variable>

#define PLATFORM_SPECIFIC_CAPTURE(...) [__VA_ARGS__]

#ifdef UNITTEST_TIME_INSENSITIVE

#define ASSERT_LE(val1, val2) do {} while(0)

#endif

using namespace ffbase;

TEST(MessageLoop, GetCurrent) {
    std::thread thread([](){
        MessageLoop::EnsureInitializedForCurrentThread();
        ASSERT_TRUE(MessageLoop::GetCurrent().GetTaskRunner());
    });

    thread.join();
}

TEST(MessageLoop, CanRunForTime) {
    bool started = false;
    bool terminated = false;
    std::thread thread([&started, &terminated]() {
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        ASSERT_TRUE(loop.GetTaskRunner());
        loop.GetTaskRunner()->PostTask([&terminated]() {
            MessageLoop::GetCurrent().Terminate();
            terminated = true;
        });
        loop.RunForTime(TimeDelta::FromSeconds(2));
        started = true;
    });
    
    thread.join();
    ASSERT_TRUE(started);
    ASSERT_TRUE(terminated);
}

TEST(MessageLoop, RunForTime) {
    
    bool run_task1 = false;
    
    std::condition_variable condition_var;
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
        
    std::thread thread([&condition_var, &run_task1]() {
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        ASSERT_TRUE(loop.GetTaskRunner());
        
        auto begin = TimePoint::Now();
        
        loop.GetTaskRunner()->PostTask([&run_task1]() {
            run_task1 = true;
        });
        
        loop.RunForTime(TimeDelta::FromSecondsFloat(0.5));
        
        auto run_last = TimePoint::Now() - begin;
        auto run_seconds = run_last.ToSecondsFloat();
        
        ASSERT_GE(run_seconds, 0.5);
        ASSERT_LE(run_seconds, 0.6);
        
        condition_var.notify_one();
    });
    
    condition_var.wait(lock);
    ASSERT_EQ(run_task1, true);
    thread.join();
}


TEST(MessageLoop, CanRunAndTerminate) {
    bool started = false;
    bool terminated = false;
    std::thread thread([&started, &terminated]() {
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        ASSERT_TRUE(loop.GetTaskRunner());
        loop.GetTaskRunner()->PostTask([&terminated]() {
            MessageLoop::GetCurrent().Terminate();
            terminated = true;
        });
        loop.Run();
        started = true;
    });
    thread.join();
    ASSERT_TRUE(started);
    ASSERT_TRUE(terminated);
}

TEST(MessageLoop, NonDelayedTasksAreRunInOrder) {
  const size_t count = 100;
  bool started = false;
  bool terminated = false;
  std::thread thread([&started, &terminated, count]() {
    MessageLoop::EnsureInitializedForCurrentThread();
    auto& loop = MessageLoop::GetCurrent();
    size_t current = 0;
    for (size_t i = 0; i < count; i++) {
      loop.GetTaskRunner()->PostTask(
          PLATFORM_SPECIFIC_CAPTURE(&terminated, i, &current, count)() {
            ASSERT_EQ(current, i);
            current++;
            if (count == i + 1) {
              MessageLoop::GetCurrent().Terminate();
              terminated = true;
            }
          });
    }
    loop.Run();
    ASSERT_EQ(current, count);
    started = true;
  });
  thread.join();
  ASSERT_TRUE(started);
  ASSERT_TRUE(terminated);
}

TEST(MessageLoop, DelayedTasksAtSameTimeAreRunInOrder) {
  const size_t count = 100;
  bool started = false;
  bool terminated = false;
  std::thread thread([&started, &terminated, count]() {
    MessageLoop::EnsureInitializedForCurrentThread();
    auto& loop = MessageLoop::GetCurrent();
    size_t current = 0;
    const auto now_plus_some =
        TimePoint::Now() + TimeDelta::FromMilliseconds(2);
    for (size_t i = 0; i < count; i++) {
      loop.GetTaskRunner()->PostTaskForTime(
          PLATFORM_SPECIFIC_CAPTURE(&terminated, i, &current, count)() {
            ASSERT_EQ(current, i);
            current++;
            if (count == i + 1) {
              MessageLoop::GetCurrent().Terminate();
              terminated = true;
            }
          },
          now_plus_some);
    }
    loop.Run();
    ASSERT_EQ(current, count);
    started = true;
  });
  thread.join();
  ASSERT_TRUE(started);
  ASSERT_TRUE(terminated);
}


TEST(MessageLoop, CheckRunsTaskOnCurrentThread) {
    std::shared_ptr<TaskRunner> runner;
    std::condition_variable condition_var;
    std::mutex mutex;
    std::unique_lock<std::mutex> lock(mutex);
    std::thread thread([&runner, &condition_var]() {
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        runner = loop.GetTaskRunner();
        condition_var.notify_one();
        ASSERT_TRUE(loop.GetTaskRunner()->RunTasksOnCurrentThread());
    });
    condition_var.wait(lock);
    ASSERT_TRUE(runner);
    ASSERT_FALSE(runner->RunTasksOnCurrentThread());
    thread.join();
}

TEST(MessageLoop, TimeSensitiveSingleDelayedTaskByDelta) {
    bool checked = false;
    std::thread thread([&checked]() {
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        auto begin = TimePoint::Now();
        loop.GetTaskRunner()->PostDelayTask(
                [begin, &checked]() {
                    auto delta = TimePoint::Now() - begin;
                    auto ms = delta.ToMillisecondsFloat();

                    FF_LOG(INFO)<<"task delay 50ms actual delay " << ms << " ms";

                    ASSERT_GE(ms, 50);
                    ASSERT_LE(ms, 80);
                    checked = true;
                    MessageLoop::GetCurrent().Terminate();
                },
                TimeDelta::FromMilliseconds(50));
        loop.Run();
    });
    thread.join();
    ASSERT_TRUE(checked);
}

TEST(MessageLoop, TimeSensitiveSingleDelayedTaskForTime) {
    bool checked = false;
    std::thread thread([&checked]() {
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        auto begin = TimePoint::Now();
        loop.GetTaskRunner()->PostTaskForTime(
                [begin, &checked]() {
                    auto delta = TimePoint::Now() - begin;
                    auto ms = delta.ToMillisecondsFloat();

                    FF_LOG(INFO)<<"task delay 50ms actual delay " << ms << " ms";

                    ASSERT_GE(ms, 50);
                    ASSERT_LE(ms, 80);
                    checked = true;
                    MessageLoop::GetCurrent().Terminate();
                },
                TimePoint::Now() + TimeDelta::FromMilliseconds(50));
        loop.Run();
    });
    thread.join();
    ASSERT_TRUE(checked);
}

TEST(MessageLoop, TimeSensitiveMultipleDelayedTasksWithIncreasingDeltas) {
    const auto count = 10;
    int checked = false;
    std::thread thread(PLATFORM_SPECIFIC_CAPTURE(&checked)() {
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        for (int target_ms = 0 + 2; target_ms < count + 2; target_ms++) {
            auto begin = TimePoint::Now();
            loop.GetTaskRunner()->PostDelayTask(
                    PLATFORM_SPECIFIC_CAPTURE(begin, target_ms, &checked)() {
                        auto delta = TimePoint::Now() - begin;
                        auto ms = delta.ToMillisecondsFloat();
                        ASSERT_GE(ms, target_ms * 20 - 20);
                        ASSERT_LE(ms, target_ms * 20 + 20);
                        checked++;
                        if (checked == count) {
                            MessageLoop::GetCurrent().Terminate();
                        }
                    },
                    TimeDelta::FromMilliseconds(target_ms * 20));
        }
        loop.Run();
    });
    thread.join();
    ASSERT_EQ(checked, count);
}

TEST(MessageLoop, TimeSensitiveMultipleDelayedTasksWithDecreasingDeltas) {
    const auto count = 10;
    int checked = false;
    std::thread thread(PLATFORM_SPECIFIC_CAPTURE(&checked)() {
        MessageLoop::EnsureInitializedForCurrentThread();
        auto& loop = MessageLoop::GetCurrent();
        for (int target_ms = count + 2; target_ms > 0 + 2; target_ms--) {
            auto begin = TimePoint::Now();
            loop.GetTaskRunner()->PostDelayTask(
                    PLATFORM_SPECIFIC_CAPTURE(begin, target_ms, &checked)() {
                        auto delta = TimePoint::Now() - begin;
                        auto ms = delta.ToMillisecondsFloat();

                        FF_LOG(INFO)<<"task delay "<< target_ms <<" ms actual delay " << ms << " ms";

                        ASSERT_GE(ms, target_ms - 2);
                        ASSERT_LE(ms, target_ms + 2);
                        checked++;
                        if (checked == count) {
                            MessageLoop::GetCurrent().Terminate();
                        }
                    },
                    TimeDelta::FromMilliseconds(target_ms));
        }
        loop.Run();
    });
    thread.join();
    ASSERT_EQ(checked, count);
}

TEST(MessageLoop, TaskObserverFire) {
    bool started = false;
    bool terminated = false;
    std::thread thread([&started, &terminated]() {
        MessageLoop::EnsureInitializedForCurrentThread();
        const size_t count = 25;
        auto& loop = MessageLoop::GetCurrent();
        size_t task_count = 0;
        size_t observe_count = 0;
        auto observer = PLATFORM_SPECIFIC_CAPTURE(&observe_count)() { observe_count++; };
        for (size_t i = 0; i < count; i++) {
            loop.GetTaskRunner()->PostTask(
                    PLATFORM_SPECIFIC_CAPTURE(&terminated, i, &task_count)() {
                        ASSERT_EQ(task_count, i);
                        task_count++;
                        if (count == i + 1) {
                            MessageLoop::GetCurrent().Terminate();
                            terminated = true;
                        }
                    });
        }
        loop.AddTaskObserver(0, observer);
        loop.Run();
        ASSERT_EQ(task_count, count);
        ASSERT_EQ(observe_count, count);
        started = true;
    });
    thread.join();
    ASSERT_TRUE(started);
    ASSERT_TRUE(terminated);
}



