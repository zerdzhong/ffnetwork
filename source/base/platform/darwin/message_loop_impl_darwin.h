//
// Created by zerdzhong on 2019/10/4.
//

#ifndef FFBASE_MESSAGE_LOOP_IMPL_DARWIN_H
#define FFBASE_MESSAGE_LOOP_IMPL_DARWIN_H

#include <CoreFoundation/CoreFoundation.h>
#include <atomic>

#include "macros.h"
#include "messageloop/message_loop_impl.h"
#include "platform/darwin/cf_reference_utils.h"

namespace ffbase {

class MessageLoopDarwin : public MessageLoopImpl {
public:
  MessageLoopDarwin();
  ~MessageLoopDarwin() override;

private:
  std::atomic_bool running_;

  CFRef<CFRunLoopRef> loop_;
  CFRef<CFRunLoopTimerRef> delay_wake_timer_;

  void Run() override;
  void Terminate() override;
  void RunForTime(TimeDelta duration) override;

  void WakeUp(TimePoint time_point) override;

  static void OnTimerFire(CFRunLoopTimerRef timer, MessageLoopDarwin *loop);

  FF_DISALLOW_COPY_AND_ASSIGN(MessageLoopDarwin);
};

} // namespace ffbase

#endif // FFBASE_MESSAGE_LOOP_IMPL_DARWIN_H
