#ifndef FFNETWORK_REQUEST_TASK_IMPL_H
#define FFNETWORK_REQUEST_TASK_IMPL_H

#include "curl/curl.h"
#include <ffnetwork/client.h>
#include <ffnetwork/request_task.h>
#include <ffnetwork/request_task_delegate.h>
#include "response_impl.h"
#include <memory>

namespace ffnetwork {

class RequestTaskInternalDelegate {
public:
  virtual void RequestTaskCancel(const std::shared_ptr<RequestTask> &task) = 0;
  virtual void RequestTaskStart(const std::shared_ptr<RequestTask> &task) = 0;
};

class RequestTaskImpl : public RequestTask,
                        public std::enable_shared_from_this<RequestTaskImpl> {

  struct HandleInfo {
    CURL *handle;
    std::string request_hash;
    curl_slist *request_headers;

    explicit HandleInfo(const std::shared_ptr<Request>& req, RequestTaskImpl* task);
    ~HandleInfo();

    void ConstructHeaders(const std::shared_ptr<Request>& request);
    void ConstructCurlHandle(const std::shared_ptr<Request>& request, RequestTaskImpl* task);
  };

public:
  RequestTaskImpl(const std::shared_ptr<Request>& request,
                  std::weak_ptr<RequestTaskInternalDelegate> delegate);
  virtual ~RequestTaskImpl();

  std::string taskIdentifier() const override;
  void setCompletionCallback(CompletionCallback completionCallback);
  void setTaskDelegate(std::weak_ptr<RequestTaskDelegate> delegate) override ;

  void Resume() override;

  void Cancel() override;
  bool isCancelled() override;

  void DidCancelled();
  void DidFinished(HttpStatusCode http_code, ResponseCode response_code);

  void OnReceiveData(char *data, size_t length);
  void OnReceiveHeader(char *data, size_t length);

  CURL *handle();

private:
  void FillMetrics();

private:
  const std::shared_ptr<Request> request_;
  const std::shared_ptr<Metrics> metrics_;
							
  std::weak_ptr<RequestTaskInternalDelegate> internal_delegate_;
  std::weak_ptr<RequestTaskDelegate> delegate_;
  std::unique_ptr<HandleInfo> handle_;
  std::shared_ptr<ResponseImpl> response_;

  const std::string identifier_;

  CompletionCallback completion_callback_;
  bool cancelled_;

  // Curl callbacks
public:
  static size_t write_callback(char *data, size_t size, size_t nitems,
                               void *str);
  static size_t header_callback(char *data, size_t size, size_t nitems,
                                void *str);
};
} // end of namespace ffnetwork

#endif
