//
// Created by zerdzhong on 2019/9/4.
//

#include "gtest/gtest.h"
#include "ffnetwork/client.h"

using namespace ffnetwork;

TEST(CurlClientTests, create) {
    auto client = createClient();
    EXPECT_NE((void *)0, client.get());
}

