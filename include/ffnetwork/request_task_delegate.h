#ifndef FFNETWORK_REQUEST_TASK_DELEGATE_H
#define FFNETWORK_REQUEST_TASK_DELEGATE_H

#include <ffnetwork/request.h>
#include <ffnetwork/response.h>

namespace ffnetwork {
class RequestTask;
class RequestTaskDelegate {
public:
  virtual void
  OnReceiveResponse(ffnetwork::RequestTask *task,
                    std::shared_ptr<ffnetwork::Response> response) = 0;
  virtual void OnReceiveData(ffnetwork::RequestTask *task, char *data,
                             size_t length) = 0;
  virtual void
  OnRequestTaskComplete(ffnetwork::RequestTask *task,
                        std::shared_ptr<ffnetwork::Response> response,
                        ffnetwork::ErrorCode error_code) = 0;
};
} // end of namespace ffnetwork

#endif