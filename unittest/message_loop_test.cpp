//
// Created by zerdzhong on 2019/10/7.
//

#include "gtest/gtest.h"
#include "base/messageloop/message_loop.h"
#include <thread>

using namespace ffbase;

TEST(MessageLoop, GetCurrent) {
    std::thread thread([](){
        MessageLoop::EnsureInitializedForCurrentThread();
        ASSERT_TRUE(MessageLoop::GetCurrent().GetTaskRunner());
    });
    
    thread.join();
}

