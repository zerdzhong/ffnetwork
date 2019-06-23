#ifndef FFNETWORK_REQUESTTASK_H
#define FFNETWORK_REQUESTTASK_H

#include <string>
#include <memory>
#include <ffnetwork/Request.h>


namespace ffnetwork {
    class RequestTask {
    public:
        virtual std::string taskIdentifier() const = 0;
        virtual void cancel() = 0;
        virtual bool cancelled() = 0;
    };
}// end of namespcae ffnetwork

#endif