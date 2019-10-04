//
// Created by zerdzhong on 2019/10/4.
//

#include "base/time/time_delta.h"
#include "gtest/gtest.h"

using namespace ffbase;


TEST(TimeDelta, Control) {
    EXPECT_LT(TimeDelta::Min(), TimeDelta::Zero());
    EXPECT_GT(TimeDelta::Max(), TimeDelta::Zero());

    EXPECT_GT(TimeDelta::Zero(), TimeDelta::FromMilliseconds(-100));
    EXPECT_LT(TimeDelta::Zero(), TimeDelta::FromMilliseconds(100));

    EXPECT_EQ(TimeDelta::FromMilliseconds(1000), TimeDelta::FromSeconds(1));
    EXPECT_EQ(TimeDelta::FromMicroseconds(1000 * 1000), TimeDelta::FromSeconds(1));
    EXPECT_EQ(TimeDelta::FromNanoseconds(1000 * 1000 * 1000), TimeDelta::FromSeconds(1));
}


