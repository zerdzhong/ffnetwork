//
// Created by zerdzhong on 2019/10/4.
//

#ifndef FFBASE_MESSAGE_LOOP_IMPL_DARWIN_H
#define FFBASE_MESSAGE_LOOP_IMPL_DARWIN_H

#include <CoreFoundation/CoreFoundation.h>
#include <atomic>

#include "message_loop_impl.h"
#include "macros.h"

namespace ffbase {

class MessageLoopDarwin : public MessageLoopImpl {
public:
    MessageLoopDarwin();
    ~MessageLoopDarwin() override;
    
private:
    std::atomic_bool running_;
    
    void Run() override;
    void Terminate() override;
    
    void WakeUp(TimePoint time_point) override;

    FF_DISALLOW_COPY_AND_ASSIGN(MessageLoopDarwin);
};

}


#endif //FFBASE_MESSAGE_LOOP_IMPL_DARWIN_H
