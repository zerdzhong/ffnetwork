#ifndef FFBASE_TIMERFD_H
#define FFBASE_TIMERFD_H

#include "time/time_point.h"

#if __has_include(<sys/timerfd.h>) && \
    (!defined(__ANDROID_API__) || __ANDROID_API_ >= 19)

#include <sys/timerfd.h>

#define FF_TIMERFD_AVAIABLE = 1

#else

#define FF_TIMERFD_AVAIABLE = 0
#include <sys/types.h>
#include <linux/time.h>

#define TFD_TIMER_ABSTIME (1<<0)
#define TFD_TIMER_CANCEL_ON_SET (1<<1)

#define TFD_CLOEXEC O_CLOEXEC
#define TFD_NONBLOCK O_NONBLOCK

int timerfd_create(int clockid, int flags);
int timerfd_settime(int ufc, int flag, const struct itimerspec* utmr, struct itimerspec* otmr);

#endif

namespace ffbase {

bool TimerRearm(int fd, TimePoint time_point);
bool TimerDrain(int fd);

}

#endif
