//
// Created by zerdzhong on 2019/10/4.
//

#include "task_runner.h"

namespace ffbase {

TaskRunner::TaskRunner(std::shared_ptr<MessageLoopImpl> loop) : loop_(loop) {
    
}

TaskRunner::~TaskRunner() = default;


}
