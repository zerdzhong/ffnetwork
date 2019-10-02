//
// Created by zerdzhong on 2019/10/2.
//

#include "base/message.h"
#include "gtest/gtest.h"

using namespace ffbase;

struct TestStruct {
    int a = 1;
    char b = 'a';
    float c = 1.2f;
};

TEST(MessageTest, CanEncodeTriviallyCopyableTypes) {
    Message message;
    ASSERT_TRUE(message.Encode(12));
    ASSERT_TRUE(message.Encode(11.0f));
    ASSERT_TRUE(message.Encode('a'));

    TestStruct s;
    ASSERT_TRUE(message.Encode(s));
    ASSERT_GE(message.GetDataLength(), 0u);
    ASSERT_GE(message.GetBufferSize(), 0u);
    ASSERT_EQ(message.GetSizeRead(), 0u);
}

