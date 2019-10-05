//
// Created by zerdzhong on 2019/10/4.
//

#ifndef FFBASE_MESSAGE_LOOP_IMPL_H
#define FFBASE_MESSAGE_LOOP_IMPL_H

#include <memory>
#include <functional>
#include <cstdint>
#include "macros.h"
#include "time/time_point.h"

namespace ffbase {

class MessageLoopImpl {
public:
    static std::shared_ptr<MessageLoopImpl>Create();
    virtual ~MessageLoopImpl();
    
    virtual void Run() = 0;
    virtual void Terminate() = 0;
    
    void PostTask(std::function<void()> task, TimePoint target_time);
    void AddTaskObserver(intptr_t key, std::function<void()> callback);
    void RemoveTaskObserver(intptr_t key);
    
    void DoRun();
    void DoTerminate();
    
protected:
    MessageLoopImpl();
    
private:
    std::atomic_bool terminated_;
    FF_DISALLOW_COPY_AND_ASSIGN(MessageLoopImpl);
};

}

#endif //FFBASE_MESSAGE_LOOP_IMPL_H
