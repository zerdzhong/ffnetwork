//
// Created by zerdzhong on 2019/10/6.
//

#include "base/thread/thread_local.h"
#include "gtest/gtest.h"

#include <atomic>
#include <thread>

using namespace ffbase;

class Box {
public:
  Box(int value, std::atomic_int *destroys = nullptr)
      : value_(value), destroys_(destroys) {}
  ~Box() {
    if (destroys_) {
      ++*destroys_;
    }
  }

  int value() const { return value_; }

private:
  int value_;
  std::atomic_int *destroys_;

  FF_DISALLOW_COPY_AND_ASSIGN(Box);
};

FF_THREAD_LOCAL ThreadLocalUniquePtr<Box> local;

TEST(ThreadLocal, Init) {
  std::thread thread([&] {
    ASSERT_EQ(local.get(), nullptr);
    auto value = 123;
    local.reset(new Box(value));
    ASSERT_EQ(local.get()->value(), value);

    std::thread thread2([&]() { ASSERT_EQ(local.get(), nullptr); });
    thread2.join();
  });
  thread.join();
}

TEST(ThreadLocal, DestroyCallback) {
  std::atomic_int destroys{0};
  std::thread thread([&] {
    ASSERT_EQ(local.get(), nullptr);
    auto value = 123;
    local.reset(new Box(value, &destroys));
    ASSERT_EQ(local.get()->value(), value);
    ASSERT_EQ(destroys.load(), 0);
  });
  thread.join();
  ASSERT_EQ(destroys.load(), 1);
}

TEST(ThreadLocal, DestroyCallback2) {
  std::atomic_int destroys{0};
  std::thread thread([&] {
    local.reset(new Box(0, &destroys));
    ASSERT_EQ(local.get()->value(), 0);
    ASSERT_EQ(destroys.load(), 0);
    local.reset(new Box(1, &destroys));
    ASSERT_EQ(local.get()->value(), 1);
    ASSERT_EQ(destroys.load(), 1);
    local.reset(new Box(2, &destroys));
    ASSERT_EQ(local.get()->value(), 2);
    ASSERT_EQ(destroys.load(), 2);
  });
  thread.join();
  ASSERT_EQ(destroys.load(), 3);
}

TEST(ThreadLocal, DestroyThreadTimeline) {
  std::atomic_int destroys{0};
  std::thread thread([&] {
    std::thread thread2([&]() {
      local.reset(new Box(100, &destroys));
      ASSERT_EQ(local.get()->value(), 100);
      ASSERT_EQ(destroys.load(), 0);
      local.reset(new Box(200, &destroys));
      ASSERT_EQ(local.get()->value(), 200);
      ASSERT_EQ(destroys.load(), 1);

      std::thread thread3([&]() {
        local.reset(new Box(2, &destroys));
        ASSERT_EQ(local.get()->value(), 2);
        ASSERT_EQ(destroys.load(), 1);
      });
      thread3.join();
      ASSERT_EQ(destroys.load(), 2);
    });
    ASSERT_EQ(local.get(), nullptr);
    thread2.join();
    ASSERT_EQ(local.get(), nullptr);
    ASSERT_EQ(destroys.load(), 3);
  });
  thread.join();
  ASSERT_EQ(destroys.load(), 3);
}
