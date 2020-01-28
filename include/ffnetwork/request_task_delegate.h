#ifndef FFNETWORK_REQUEST_TASK_DELEGATE_H
#define FFNETWORK_REQUEST_TASK_DELEGATE_H

#include <ffnetwork/request.h>

namespace ffnetwork {
class RequestTask;

class RequestTaskDelegate {
public:
  virtual void RequestTaskError(RequestTask *task) = 0;
  virtual bool ShouldPerformRedirect(RequestTask *task, Request *new_request) {
    return true;
  }
};

} // end of namespace ffnetwork

#endif