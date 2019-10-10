//
// Created by zerdzhong on 2019/10/9.
//

#ifndef FFBASE_MESSAGE_LOOP_IMPL_LINUX_H
#define FFBASE_MESSAGE_LOOP_IMPL_LINUX_H

#include "macros.h"
#include "message_loop_impl.h"

namespace ffbase {

class MessageLoopImplLinux : public MessageLoopImpl {
public:
    MessageLoopImplLinux();
    ~MessageLoopImplLinux() override;

private:
    void Run() override;
    void Terminate() override;
    void WakeUp(TimePoint time_point) override;

    FF_DISALLOW_COPY_AND_ASSIGN(MessageLoopImplLinux);
};

}
#endif //FFNETWORK_MESSAGE_LOOP_IMPL_LINUX_H
