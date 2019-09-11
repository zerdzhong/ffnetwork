#include "request_task_impl.h"

namespace ffnetwork {
    RequestTaskImpl::RequestTaskImpl(const std::weak_ptr<RequestTaskDelegate> &delegate,
                                     const std::string &identifier) :
            cancelled_(false),
            identifier_(identifier),
            delegate_(delegate)
    {

    }

    RequestTaskImpl::~RequestTaskImpl() {

    }

    std::string RequestTaskImpl::taskIdentifier() const {
        return identifier_;
    }

    void RequestTaskImpl::cancel() {
        cancelled_ = true;
        if (auto delegate = delegate_.lock()) {
            delegate->RequestTaskDidCancel(shared_from_this());
        }
    }

    bool RequestTaskImpl::cancelled() {
        return cancelled_;
    }

}
