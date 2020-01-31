#ifndef FFNETWORK_REQUEST_TASK_DELEGATE_H
#define FFNETWORK_REQUEST_TASK_DELEGATE_H

#include <ffnetwork/request.h>
#include <ffnetwork/response.h>

namespace ffnetwork {
class RequestTask;

class RequestTaskDelegate {
public:
  virtual void OnReceiveResponse(RequestTask *task, std::shared_ptr<Response> response) = 0;
  virtual void OnReceiveData(RequestTask *task, char *data, size_t length) = 0;
  virtual void OnRequestTaskComplete(RequestTask *task, ResponseCode result) = 0;
};
} // end of namespace ffnetwork

#endif