//
// Created by zerdzhong on 2019/10/4.
//

#include "message_loop_impl_darwin.h"
#include <CoreFoundation/CFRunLoop.h>
#include "logging.h"

namespace ffbase {

static constexpr CFTimeInterval kDistantFuture = 1.0e10;

MessageLoopDarwin::MessageLoopDarwin() :
running_(false),
loop_((CFRunLoopRef)CFRetain(CFRunLoopGetCurrent()))
{
    FF_CHECK(loop_ != nullptr);
    
    CFRunLoopTimerContext timer_context = {
        .info = this,
    };
    
    delay_wake_timer_.Reset(CFRunLoopTimerCreate(kCFAllocatorDefault,
                                                 kDistantFuture /* fire date */,
                                                 HUGE_VAL /* interval */,
                                                 0 /* flags */,
                                                 0 /* order */,
                                                 reinterpret_cast<CFRunLoopTimerCallBack>(&MessageLoopDarwin::OnTimerFire),
                                                 &timer_context));
    
    FF_CHECK(delay_wake_timer_ != nullptr);
    
    CFRunLoopAddTimer(loop_, delay_wake_timer_, kCFRunLoopCommonModes);
}

MessageLoopDarwin::~MessageLoopDarwin() {
    CFRunLoopTimerInvalidate(delay_wake_timer_);
    CFRunLoopRemoveTimer(loop_, delay_wake_timer_, kCFRunLoopCommonModes);
}

void MessageLoopDarwin::Run() {
    FF_CHECK(loop_ == CFRunLoopGetCurrent());
    
    running_ = true;
    
    while (running_) {
        @autoreleasepool {
            int result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, kDistantFuture, YES);
            if (result == kCFRunLoopRunStopped || result == kCFRunLoopRunFinished) {
                @autoreleasepool {
                    RunExpiredTasksNow();
                }
                
                running_ = false;
            }
        }
    }
}

void MessageLoopDarwin::Terminate() {
    running_ = false;
    CFRunLoopStop(loop_);
}

void MessageLoopDarwin::WakeUp(TimePoint time_point) {
    CFRunLoopTimerSetNextFireDate(
                                  delay_wake_timer_,
                                  CFAbsoluteTimeGetCurrent() + (time_point - TimePoint::Now()).ToSecondsFloat());
}

void MessageLoopDarwin::OnTimerFire(CFRunLoopTimerRef timer, MessageLoopDarwin* loop) {
    @autoreleasepool {
        // RunExpiredTasksNow rearms the timer as appropriate via a call to WakeUp.
        loop->RunExpiredTasksNow();
    }
}

}//end of namespace ffbase
