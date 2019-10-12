//
// Created by zerdzhong on 2019/10/7.
//

#include "gtest/gtest.h"
#include "base/messageloop/message_loop.h"
#include "time/time_point.h"
#include <thread>

#define PLATFORM_SPECIFIC_CAPTURE(...) [__VA_ARGS__]

using namespace ffbase;

TEST(MessageLoop, GetCurrent) {
    std::thread thread([](){
        MessageLoop::EnsureInitializedForCurrentThread();
        ASSERT_TRUE(MessageLoop::GetCurrent().GetTaskRunner());
    });

    thread.join();
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

