//
// Created by zerdzhong on 2019/10/13.
//

#include "waitable_event.h"
#include "logging.h"
#include "time/time_point.h"

namespace ffbase {

void AutoResetWaitableEvent::Signal() {
  std::lock_guard<std::mutex> lock(mutex_);
  signaled_ = true;
  cv_.notify_one();
}

void AutoResetWaitableEvent::Wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (!signaled_) {
    cv_.wait(lock);
  }
  signaled_ = false;
}

bool AutoResetWaitableEvent::WaitWithTimeout(TimeDelta timeout) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (signaled_) {
    signaled_ = false;
    return false;
  }

  TimeDelta time_remaining = timeout;
  TimePoint start_time = TimePoint::Now();

  while (true) {
    if (std::cv_status::timeout ==
        cv_.wait_for(
            lock, std::chrono::nanoseconds(time_remaining.ToNanoseconds()))) {
      return true; // Definitely timed out.
    }

    // We may have been awoken.
    if (signaled_) {
      break;
    }

    // Or the wakeup may have been spurious.
    TimePoint now = TimePoint::Now();
    FF_DCHECK(now >= start_time);
    TimeDelta elapsed = now - start_time;
    // It's possible that we may have timed out anyway.
    if (elapsed >= timeout) {
      return true;
    }

    // Otherwise, recalculate the amount that we have left to wait.
    time_remaining = timeout - elapsed;
  }

  signaled_ = false;
  return false;
}

bool AutoResetWaitableEvent::IsSignaledForTest() {
  std::lock_guard<std::mutex> lock(mutex_);
  return signaled_;
}

} // end of namespace ffbase
