//
// Created by zerdzhong on 2019/10/13.
//

#include "gtest/gtest.h"

#define private public
#include "base/thread/thread.h"
#undef private

using namespace ffbase;

TEST(Thread, Basic) {
    Thread thread;
    ASSERT_TRUE(thread.GetTaskRunner());
}

TEST(Thread, SetName) {
    std::string set_name = "test-thread-name";
    Thread thread(set_name);
    EXPECT_EQ(thread.GetName(), set_name);
    thread.GetTaskRunner()->PostTask([&set_name] {
        char* pthread_name = (char *)malloc(set_name.length() + 1);
        pthread_getname_np(pthread_self(), pthread_name, set_name.length()+1);
        EXPECT_EQ(std::string(pthread_name), set_name);
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
