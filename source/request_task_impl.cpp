#include "request_task_impl.h"

namespace ffnetwork {
    RequestTaskImpl::RequestTaskImpl(const std::weak_ptr<RequestTaskDelegate> &delegate,
                                     const std::string &identifier) :
            cancelled_(false),
            delegate_(delegate),
            identifier_(identifier)
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
            delegate->requestTaskDidCancel(shared_from_this());
        }
    }

    bool RequestTaskImpl::cancelled() {
        return cancelled_;
    }

}
