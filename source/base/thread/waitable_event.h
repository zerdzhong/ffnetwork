//
// Created by zerdzhong on 2019/10/13.
//

#ifndef FFBASE_WAITABLE_EVENT_H
#define FFBASE_WAITABLE_EVENT_H

#include "macros.h"
#include "thread_annotations.h"
#include "time/time_delta.h"

#include <condition_variable>
#include <mutex>

namespace ffbase {

// An event that can be signaled and waited on. This version automatically
// returns to the unsignaled state after unblocking one waiter.
// This class is thread-safe.
class AutoResetWaitableEvent final {
public:
  AutoResetWaitableEvent() {}
  ~AutoResetWaitableEvent() {}

  // Put the event in the signaled state. Exactly one |Wait()| will be unblocked
  // and the event will be returned to the unsignaled state.
  void Signal();

  // Blocks the calling thread until the event is signaled. Upon unblocking, the
  // event is returned to the unsignaled state
  void Wait();

  // Like |Wait()|, but with a timeout. Also unblocks if |timeout_microseconds|
  // without being signaled in which case it returns true (otherwise, it returns
  // false).
  bool WaitWithTimeout(TimeDelta timeout);

  bool IsSignaledForTest();

private:
  std::condition_variable cv_;
  std::mutex mutex_;
  // True if this event is in the signaled state.
  bool signaled_ = false;

  FF_DISALLOW_COPY_AND_ASSIGN(AutoResetWaitableEvent);
};

} // end of namespace ffbase

#endif // FFNETWORK_WAITABLE_EVENT_H
