//
// Created by zerdzhong on 2019/10/13.
//

#include "gtest/gtest.h"
#include <pthread.h>
#define private public
#include "base/thread/thread.h"
#undef private

using namespace ffbase;

TEST(Thread, Basic) {
    Thread thread;
    ASSERT_TRUE(thread.GetTaskRunner());
}

TEST(Thread, SetName) {
    std::string set_name = "test-name";
    Thread thread(set_name);
    EXPECT_EQ(thread.GetName(), set_name);
    thread.GetTaskRunner()->PostTask([&set_name] {
        char pthread_name[16];
        memset(pthread_name, 0x00, 16);
        pthread_getname_np(pthread_self(), pthread_name, 16);
        std::string thread_name_str(pthread_name);
        EXPECT_EQ(thread_name_str, set_name);
    });
}

TEST(Thread, CanStartAndEndWithExplicitJoin) {
    Thread thread;
    ASSERT_TRUE(thread.GetTaskRunner());
    thread.Join();
}

TEST(Thread, HasARunningMessageLoop) {
    Thread thread;
    bool done = false;
    thread.GetTaskRunner()->PostTask([&done]() { done = true; });
    thread.Join();
    ASSERT_TRUE(done);
}
