#ifndef FFNETWORK_REQUEST_TASK_DELEGATE_H
#define FFNETWORK_REQUEST_TASK_DELEGATE_H

#include <ffnetwork/request_task.h>

namespace ffnetwork {
    class RequestTaskDelegate {
    public:
        virtual void RequestTaskDidCancel(const std::shared_ptr<RequestTask> &task) = 0;
    };
}//end of namespace ffnetwork

#endif