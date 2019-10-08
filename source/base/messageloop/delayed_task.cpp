//
// Created by zhongzhendong on 2019/10/6.
//

#include "delayed_task.h"

namespace ffbase {

DelayedTask::DelayedTask(size_t order, closure task, TimePoint target_time) :
order_(order),
task_(task),
target_time_(target_time)
{
    
}

DelayedTask::DelayedTask(const DelayedTask& other) = default;
DelayedTask::~DelayedTask() = default;

const closure& DelayedTask::GetTask() const {
    return task_;
}

TimePoint DelayedTask::GetTargetTime() const {
    return target_time_;
}

bool DelayedTask::operator>(const DelayedTask& other) const {
    if (target_time_ == other.target_time_) {
      return order_ > other.order_;
    }
    return target_time_ > other.target_time_;
}

}
