#ifndef FFNETWORK_REQUESTTASKDELEGATE_H
#define FFNETWORK_REQUESTTASKDELEGATE_H

#include <ffnetwork/RequestTask.h>

namespace ffnetwork {
    class RequestTaskDelegate {
    public:
        virtual void requestTaskDidCancel(const RequestTask* task) const = 0;
    };
}//end of namespace ffnetwork

#endif