//
// Created by zerdzhong on 2019/10/4.
//

#ifndef FFBASE_TASK_RUNNER_H
#define FFBASE_TASK_RUNNER_H

#include "macros.h"
#include "time/time_point.h"
#include <functional>
#include <memory>

namespace ffbase {

using closure = std::function<void()>;
class MessageLoopImpl;

class TaskRunner {
public:
    virtual ~TaskRunner();
    virtual void PostTask(closure task);
    virtual void PostTaskForTime(closure task, TimePoint target_time);
    virtual void PostDelayTask(closure task, TimeDelta delay);
    
    virtual bool RunTasksOnCurrentThread();

    explicit TaskRunner(std::shared_ptr<MessageLoopImpl> loop);
    
    static void RunNowOrPostTask(const std::shared_ptr<TaskRunner>& runner, closure task);
    
private:
    std::shared_ptr<MessageLoopImpl> loop_;
    FF_DISALLOW_COPY_AND_ASSIGN(TaskRunner);
};


}

#endif //FFBASE_TASK_RUNNER_H
