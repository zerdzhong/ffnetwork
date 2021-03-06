//
// Created by zerdzhong on 2019/10/9.
//

#ifndef FFBASE_MESSAGE_LOOP_IMPL_LINUX_H
#define FFBASE_MESSAGE_LOOP_IMPL_LINUX_H

#include "macros.h"
#include "message_loop_impl.h"
#include "unique_fd.h"
#include <atomic>

namespace ffbase {

class MessageLoopImplLinux : public MessageLoopImpl {
public:
  MessageLoopImplLinux();
  ~MessageLoopImplLinux() override;

private:
  UniqueFD epoll_fd_;
  UniqueFD timer_fd_;

  std::atomic_bool running_;

  void Run() override;
  void Terminate() override;
  void RunForTime(TimeDelta duration) override;
  void WakeUp(TimePoint time_point) override;

  void OnEventFired();
  bool AddOrRemoveTimerSource(bool add);

  FF_DISALLOW_COPY_AND_ASSIGN(MessageLoopImplLinux);
};

} // namespace ffbase
#endif // FFNETWORK_MESSAGE_LOOP_IMPL_LINUX_H
