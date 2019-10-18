//
// Created by zerdzhong on 2019/10/4.
//

#include "base/time/time_point.h"
#include "gtest/gtest.h"
#include <thread>


using namespace ffbase;

TEST(Time, Now) {
    auto start = TimePoint::Now();
    for (int i = 0; i < 3; ++i) {
        auto now = TimePoint::Now();
        EXPECT_GE(now, start);
        std::this_thread::yield();
    }
}

TEST(Time, IntConversions) {
    // Integer conversions should all truncate, not round.
    TimeDelta delta = TimeDelta::FromNanoseconds(102304506708ll);
    EXPECT_EQ(102304506708ll, delta.ToNanoseconds());
    EXPECT_EQ(102304506ll, delta.ToMicroseconds());
    EXPECT_EQ(102304ll, delta.ToMilliseconds());
    EXPECT_EQ(102ll, delta.ToSeconds());
}

TEST(Time, FloatConversions) {
    // Float conversions should remain close to the original value.
    TimeDelta delta = TimeDelta::FromNanoseconds(102304506708ll);
    EXPECT_FLOAT_EQ(102304506708.0, delta.ToNanosecondsFloat());
    EXPECT_FLOAT_EQ(102304506.708, delta.ToMicrosecondsFloat());
    EXPECT_FLOAT_EQ(102304.506708, delta.ToMillisecondsFloat());
    EXPECT_FLOAT_EQ(102.304506708, delta.ToSecondsFloat());
}

TEST(Time, TimespecConversions) {
    struct timespec ts;
    ts.tv_sec = 5;
    ts.tv_nsec = 7;
    TimeDelta from_timespec = TimeDelta::FromTimespec(ts);
    EXPECT_EQ(5, from_timespec.ToSeconds());
    EXPECT_EQ(5 * 1000000000ll + 7, from_timespec.ToNanoseconds());
    struct timespec to_timespec = from_timespec.ToTimespec();
    EXPECT_EQ(ts.tv_sec, to_timespec.tv_sec);
    EXPECT_EQ(ts.tv_nsec, to_timespec.tv_nsec);
}