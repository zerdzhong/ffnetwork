//
// Created by zerdzhong on 2019/10/9.
//

#include "message_loop_impl_linux.h"
#include <sys/epoll.h>
#include <unistd.h>
#include "platform/linux/timerfd.h"
#include "logging.h"

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
    running_ = true;

    while (running_) {
        struct epoll_event event = {};

        int epoll_result = ::epoll_wait(epoll_fd_.get(), &event, 1, -1 /* timeout */);

        // Errors are fatal.
        if (event.events & (EPOLLERR | EPOLLHUP)) {
            running_ = false;
            continue;
        }

        // Timeouts are fatal since we specified an infinite timeout already.
        // Likewise, > 1 is not possible since we waited for one result.
        if (epoll_result != 1) {
            running_ = false;
            continue;
        }

        if (event.data.fd == timer_fd_.get()) {
            OnEventFired();
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
