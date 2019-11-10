//
// Created by zerdzhong on 2019/10/9.
//

#include "message_loop_impl_linux.h"
#include <sys/epoll.h>
#include <unistd.h>
#include "platform/linux/timerfd.h"
#include "time/time_point.h"
#include "logging.h"
#include <errno.h>

namespace ffbase {

static constexpr int kClockType = CLOCK_MONOTONIC;

MessageLoopImplLinux::MessageLoopImplLinux() : 
epoll_fd_(::epoll_create(1)), 
timer_fd_(::timerfd_create(kClockType, TFD_NONBLOCK | TFD_CLOEXEC))
{
    FF_CHECK(epoll_fd_.is_valid());
    FF_CHECK(timer_fd_.is_valid());
    bool added_source = AddOrRemoveTimerSource(true);
    FF_CHECK(added_source);
}
MessageLoopImplLinux::~MessageLoopImplLinux() {
    bool removed_source = AddOrRemoveTimerSource(false);
    FF_CHECK(removed_source);
}

void MessageLoopImplLinux::Run() {
    RunForTime(TimeDelta::Max());
}

void MessageLoopImplLinux::RunForTime(TimeDelta duration) {
    running_ = true;

    auto start_time = TimePoint::Now();
    auto left_milliseconds = duration.ToMilliseconds();

    while(running_) {
        struct epoll_event event = {};
        int epoll_result = ::epoll_wait(epoll_fd_.get(), &event, 1, left_milliseconds);

        //Error are fatal.
        if (event.events & (EPOLLERR | EPOLLHUP)) {
            running_ = false;
            continue;
        }

        if (epoll_result != 1 && errno != EINTR) {
            running_ = false;
            continue;
        }

        if (event.data.fd == timer_fd_.get()) {
            OnEventFired();
        }

        left_milliseconds -= (TimePoint::Now() - start_time).ToMilliseconds();
        if (left_milliseconds <= 0) {
            running_ = false;
        }
    }
}

void MessageLoopImplLinux::Terminate() {
    running_ = false;
    WakeUp(TimePoint::Now());
}

void MessageLoopImplLinux::WakeUp(TimePoint time_point) {
    bool result = TimerRearm(timer_fd_.get(), time_point);
    FF_DCHECK(result);
}

void MessageLoopImplLinux::OnEventFired() {
    if (TimerDrain(timer_fd_.get())) {
        RunExpiredTasksNow();
    }
}

bool MessageLoopImplLinux::AddOrRemoveTimerSource(bool add) {
    struct epoll_event event = {};

    event.events = EPOLLIN;
    // The data is just for informational purposes so we know when we were worken
    // by the FD.
    event.data.fd = timer_fd_.get();

    int ctl_result =
        ::epoll_ctl(epoll_fd_.get(), add ? EPOLL_CTL_ADD : EPOLL_CTL_DEL,
                  timer_fd_.get(), &event);
    return ctl_result == 0;
}

}
