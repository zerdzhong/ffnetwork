//
// Created by zerdzhong on 2019/10/4.
//

#include "message_loop_impl_darwin.h"
#include <CoreFoundation/CFRunLoop.h>
#include "time/time_point.h"
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
    RunForTime(TimeDelta::Max());
}

void MessageLoopDarwin::RunForTime(TimeDelta duration) {
    FF_CHECK(loop_ == CFRunLoopGetCurrent());
    
    running_ = true;
    auto start_time = TimePoint::Now();
    auto left_seconds = duration.ToSecondsFloat();
    
    while (running_) {
        @autoreleasepool {
            int result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, left_seconds, YES);
            if (result == kCFRunLoopRunStopped ||
                result == kCFRunLoopRunFinished ||
                result == kCFRunLoopRunTimedOut) {
                
                FF_LOG(INFO) << "runinmode return result "<< result;
                
                @autoreleasepool {
                    RunExpiredTasksNow();
                }
                
                running_ = false;
            }
            
            left_seconds -= (TimePoint::Now() - start_time).ToSecondsFloat();
            FF_LOG(INFO) << "run left seconds "<< left_seconds;
            if(left_seconds <= 0) {
                running_ = false;
            }
        }
    }

}

void MessageLoopDarwin::Terminate() {
    FF_LOG(INFO) << "Terminate messageloop";
    running_ = false;
    CFRunLoopStop(loop_);
}

void MessageLoopDarwin::WakeUp(TimePoint time_point) {
    auto wakeup_delay_seconds = (time_point - TimePoint::Now()).ToSecondsFloat();
    if (wakeup_delay_seconds < 0) {
        wakeup_delay_seconds = 0;
    }
    FF_LOG(INFO) << "WakeUp nowtime " << TimePoint::Now().ToEpochDelta().ToMilliseconds()
    <<" after "<< wakeup_delay_seconds << " seconds";
    CFRunLoopTimerSetNextFireDate(delay_wake_timer_,
                                  CFAbsoluteTimeGetCurrent() + wakeup_delay_seconds);
}

void MessageLoopDarwin::OnTimerFire(CFRunLoopTimerRef timer, MessageLoopDarwin* loop) {
    @autoreleasepool {
        // RunExpiredTasksNow rearms the timer as appropriate via a call to WakeUp.
        FF_LOG(INFO) << "OnTimerFire "<< TimePoint::Now().ToEpochDelta().ToMilliseconds();
        loop->RunExpiredTasksNow();
    }
}

}//end of namespace ffbase
