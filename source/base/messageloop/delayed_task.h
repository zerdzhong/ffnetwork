//
// Created by zerdzhong on 2019/10/6.
//

#ifndef FFBASE_DELAYED_TASK_H
#define FFBASE_DELAYED_TASK_H

#include "time/time_point.h"
#include <queue>
#include <functional>

namespace ffbase {

using closure = std::function<void()>;

class DelayedTask {
public:
    DelayedTask(size_t order, closure task, TimePoint target_time);
    DelayedTask(const DelayedTask& other);
    ~DelayedTask();
    
    const closure& GetTask() const;
    TimePoint GetTargetTime() const;
    
    bool operator>(const DelayedTask& other) const;
    
private:
    size_t order_;
    closure task_;
    TimePoint target_time_;
};

using DelayedTaskQueue = std::priority_queue<DelayedTask, std::deque<DelayedTask>, std::greater<DelayedTask>>;

}

#endif //FFBASE_DELAYED_TASK_H
