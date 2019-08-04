#ifndef FFNETWORK_REQUEST_TASK_DELEGATE_H
#define FFNETWORK_REQUEST_TASK_DELEGATE_H

#include <ffnetwork/request_task.h>

namespace ffnetwork {
    class RequestTaskDelegate {
    public:
        virtual void requestTaskDidCancel(const RequestTask* task) const = 0;
    };
}//end of namespace ffnetwork

#endif