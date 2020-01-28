#ifndef FFNETWORK_REQUEST_TASK_H
#define FFNETWORK_REQUEST_TASK_H

#include <ffnetwork/request.h>
#include <ffnetwork/request_task_delegate.h>
#include <memory>
#include <string>

namespace ffnetwork {
class RequestTask {
public:
  virtual std::string taskIdentifier() const = 0;

  virtual void Resume() = 0;

  virtual void Cancel() = 0;
  virtual bool isCancelled() = 0;
};
} // end of namespace ffnetwork

#endif