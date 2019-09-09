#ifndef FFNETWORK_REQUEST_TASK_IMPL_H
#define FFNETWORK_REQUEST_TASK_IMPL_H

#include <ffnetwork/request_task.h>
#include <ffnetwork/request_task_delegate.h>
#include <memory>

namespace ffnetwork {
class RequestTaskImpl : public RequestTask, public std::enable_shared_from_this<RequestTaskImpl> {
    public:

        RequestTaskImpl(const std::weak_ptr<RequestTaskDelegate> &delegate,
                             const std::string &identifier);

        virtual ~RequestTaskImpl();

        std::string taskIdentifier() const override;
        void cancel() override;
        bool cancelled() override;

    private:
        bool cancelled_;
        const std::string identifier_;
        const std::weak_ptr<RequestTaskDelegate> delegate_;
    };
}//end of namespace ffnetwork

#endif