#ifndef FFNETWORK_CLIENT_H
#define FFNETWORK_CLIENT_H

#include <ffnetwork/request.h>
#include <ffnetwork/request_task.h>
#include <ffnetwork/response.h>
#include <functional>
#include <memory>
#include <string>

namespace ffnetwork {
typedef std::function<void(const std::shared_ptr<Response> &)>
    CompletionCallback;
class Client {
public:
  virtual ~Client();

  virtual std::shared_ptr<RequestTask>
  TaskWithRequest(const std::shared_ptr<Request> &request,
                  CompletionCallback) = 0;

  virtual std::shared_ptr<RequestTask>
  PerformRequest(const std::shared_ptr<Request> &request,
                 CompletionCallback callback) = 0;

  virtual std::shared_ptr<Response>
  PerformRequestSync(const std::shared_ptr<Request> &request);
};

extern std::shared_ptr<Client> CreateClient();
} // namespace ffnetwork

#endif
