#ifndef FFNETWORK_REQUEST_TASK_H
#define FFNETWORK_REQUEST_TASK_H

#include <string>
#include <memory>
#include <ffnetwork/request.h>


namespace ffnetwork {
    class RequestTask {
    public:
        virtual std::string taskIdentifier() const = 0;
        virtual void cancel() = 0;
        virtual bool cancelled() = 0;
    };
}// end of namespcae ffnetwork

#endif