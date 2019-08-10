//
// Created by zerdzhong on 2019-08-10.
//
#include "gtest/gtest.h"
#include "utils/time_utils.h"

using namespace ffnetwork;

TEST(TimeTest, Intervals) {
    TimeStamp ts_earlier = NowTimeMillis();
    TimeStamp ts_later = TimeAfter(500);
    // We can't depend on ts_later and ts_earlier to be exactly 500 apart
    // since time elapses between the calls to Time() and TimeAfter(500)
    EXPECT_LE(500,  TimeDiff(ts_later, ts_earlier));
    EXPECT_GE(-500, TimeDiff(ts_earlier, ts_later));
    // Time has elapsed since ts_earlier
    EXPECT_GE(TimeSince(ts_earlier), 0);
    // ts_earlier is earlier than now, so TimeUntil ts_earlier is -ve
    EXPECT_LE(TimeUntil(ts_earlier), 0);
    // ts_later likely hasn't happened yet, so TimeSince could be -ve
    // but within 500
    EXPECT_GE(TimeSince(ts_later), -500);
    // TimeUntil ts_later is at most 500
    EXPECT_LE(TimeUntil(ts_later), 500);
}


TEST(TimeTest, BoundaryComparison) {
    // Obtain two different times, in known order
    TimeStamp ts_earlier = static_cast<TimeStamp>(-50);
    TimeStamp ts_later = ts_earlier + 100;
    EXPECT_NE(ts_earlier, ts_later);
    // Common comparisons
    EXPECT_TRUE( TimeIsLaterOrEqual(ts_earlier, ts_later));
    EXPECT_TRUE( TimeIsLater(       ts_earlier, ts_later));
    EXPECT_FALSE(TimeIsLaterOrEqual(ts_later,   ts_earlier));
    EXPECT_FALSE(TimeIsLater(       ts_later,   ts_earlier));
    // Earlier of two times
    EXPECT_EQ(ts_earlier, TimeMin(ts_earlier, ts_earlier));
    EXPECT_EQ(ts_earlier, TimeMin(ts_earlier, ts_later));
    EXPECT_EQ(ts_earlier, TimeMin(ts_later,   ts_earlier));
    // Later of two times
    EXPECT_EQ(ts_earlier, TimeMax(ts_earlier, ts_earlier));
    EXPECT_EQ(ts_later,   TimeMax(ts_earlier, ts_later));
    EXPECT_EQ(ts_later,   TimeMax(ts_later,   ts_earlier));
    // Interval
    EXPECT_EQ(100,  TimeDiff(ts_later, ts_earlier));
    EXPECT_EQ(-100, TimeDiff(ts_earlier, ts_later));
}
