#ifndef FFNETWORK_TIME_UTILS_H
#define FFNETWORK_TIME_UTILS_H

#include <time.h>
#include <stddef.h>  // for NULL, size_t
#include <stdint.h>  // for uintptr_t and (u)int_t types.

namespace ffnetwork {
    static const int64_t kNumMillisecsPerSec = INT64_C(1000);
    static const int64_t kNumMicrosecsPerSec = INT64_C(1000000);
    static const int64_t kNumNanosecsPerSec = INT64_C(1000000000);

    static const int64_t kNumMicrosecsPerMillisec =
        kNumMicrosecsPerSec / kNumMillisecsPerSec;
    static const int64_t kNumNanosecsPerMillisec =
        kNumNanosecsPerSec / kNumMillisecsPerSec;
    static const int64_t kNumNanosecsPerMicrosec =
        kNumNanosecsPerSec / kNumMicrosecsPerSec;

    // January 1970, in NTP milliseconds.
    static const int64_t kJan1970AsNtpMillisecs = INT64_C(2208988800000);

    // Returns the current time in milliseconds.
    uint32_t NowTimeMillis();
    // Returns the current time in microseconds.
    uint64_t NowTimeMicros();
    // Returns the current time in nanoseconds.
    uint64_t NowTimeNanos();

    uint32_t TimeAfter(int32_t elapsed);
    // Comparisons between time values, which can wrap around.
    bool TimeIsBetween(uint32_t earlier,
                       uint32_t middle,
                       uint32_t later);                         // Inclusive

    bool TimeIsLaterOrEqual(uint32_t earlier, uint32_t later);  // Inclusive
    bool TimeIsLater(uint32_t earlier, uint32_t later);         // Exclusive
    
    // Returns the later of two timestamps.
    inline uint32_t TimeMax(uint32_t ts1, uint32_t ts2) {
        return TimeIsLaterOrEqual(ts1, ts2) ? ts2 : ts1;
    }

    // Returns the earlier of two timestamps.
    inline uint32_t TimeMin(uint32_t ts1, uint32_t ts2) {
        return TimeIsLaterOrEqual(ts1, ts2) ? ts1 : ts2;
    }

    // Number of milliseconds that would elapse between 'earlier' and 'later'
    // timestamps.  The value is negative if 'later' occurs before 'earlier'.
    int32_t TimeDiff(uint32_t later, uint32_t earlier);
    
    // The number of milliseconds that have elapsed since 'earlier'.
    inline int32_t TimeSince(uint32_t earlier) {
        return TimeDiff(NowTimeMillis(), earlier);
    }

    // The number of milliseconds that will elapse between now and 'later'.
    inline int32_t TimeUntil(uint32_t later) {
        return TimeDiff(later, NowTimeMillis());
    }

}

#endif