//
// Created by zerdzhong on 2019/10/13.
//

#include "base/thread/waitable_event.h"
#include "gtest/gtest.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <atomic>
#include <thread>
#include <type_traits>
#include <vector>

using namespace ffbase;

TEST(AutoResetWaitableEventTest, Basic) {
  AutoResetWaitableEvent ev;
  EXPECT_FALSE(ev.IsSignaledForTest());
  ev.Signal();
  EXPECT_TRUE(ev.IsSignaledForTest());
  ev.Wait();
  EXPECT_FALSE(ev.IsSignaledForTest());

  AutoResetWaitableEvent ev1;
  EXPECT_FALSE(ev1.IsSignaledForTest());
  ev1.Signal();
  EXPECT_TRUE(ev1.IsSignaledForTest());

  AutoResetWaitableEvent ev2;
  EXPECT_FALSE(ev2.IsSignaledForTest());
  EXPECT_TRUE(ev2.WaitWithTimeout(TimeDelta::Zero()));
  EXPECT_FALSE(ev2.IsSignaledForTest());
  EXPECT_TRUE(ev2.WaitWithTimeout(TimeDelta::FromMilliseconds(1)));
  EXPECT_FALSE(ev2.IsSignaledForTest());
  ev2.Signal();
  EXPECT_TRUE(ev2.IsSignaledForTest());
  EXPECT_FALSE(ev2.WaitWithTimeout(TimeDelta::Zero()));
  EXPECT_FALSE(ev2.IsSignaledForTest());
  EXPECT_TRUE(ev2.WaitWithTimeout(TimeDelta::FromMilliseconds(1)));
  EXPECT_FALSE(ev2.IsSignaledForTest());
  ev2.Signal();
  EXPECT_FALSE(ev2.WaitWithTimeout(TimeDelta::FromMilliseconds(1)));
  EXPECT_FALSE(ev2.IsSignaledForTest());
}

constexpr TimeDelta kEpsilonTimeout = TimeDelta::FromMilliseconds(20);
constexpr TimeDelta kTinyTimeout = TimeDelta::FromMilliseconds(100);
constexpr TimeDelta kActionTimeout = TimeDelta::FromMilliseconds(10000);

// Sleeps for a "very small" amount of time.

void SleepFor(TimeDelta duration) {
  std::this_thread::sleep_for(
      std::chrono::nanoseconds(duration.ToNanoseconds()));
}

void EpsilonRandomSleep() {
  TimeDelta duration =
      TimeDelta::FromMilliseconds(static_cast<unsigned>(rand()) % 20u);
  SleepFor(duration);
}

TEST(AutoResetWaitableEventTest, MultipleWaiters) {
  AutoResetWaitableEvent ev;

  std::atomic_uint wake_count(0u);
  std::vector<std::thread> threads;
  for (size_t j = 0u; j < 4u; j++) {
    threads.push_back(std::thread([&ev, &wake_count]() {
      if (rand() % 2 == 0) {
        ev.Wait();
      } else {
        EXPECT_FALSE(ev.WaitWithTimeout(kActionTimeout));
      }
      wake_count.fetch_add(1u);
      // Note: We can't say anything about the signaled state of |ev| here,
      // since the main thread may have already signaled it again.
    }));
  }

  // Unfortunately, we can't really wait for the threads to be waiting, so we
  // just sleep for a bit, and count on them having started and advanced to
  // waiting.
  SleepFor(kTinyTimeout + kTinyTimeout);

  for (size_t j = 0u; j < threads.size(); j++) {
    unsigned old_wake_count = wake_count.load();
    EXPECT_EQ(j, old_wake_count);

    // Each |Signal()| should wake exactly one thread.
    ev.Signal();

    // Poll for |wake_count| to change.
    while (wake_count.load() == old_wake_count) {
      SleepFor(kEpsilonTimeout);
    }

    EXPECT_FALSE(ev.IsSignaledForTest());

    // And once it's changed, wait a little longer, to see if any other
    // threads are awoken (they shouldn't be).
    SleepFor(kEpsilonTimeout);

    EXPECT_EQ(old_wake_count + 1u, wake_count.load());

    EXPECT_FALSE(ev.IsSignaledForTest());
  }

  // Having done that, if we signal |ev| now, it should stay signaled.
  ev.Signal();
  SleepFor(kEpsilonTimeout);
  EXPECT_TRUE(ev.IsSignaledForTest());

  for (auto &thread : threads) {
    thread.join();
  }
}
