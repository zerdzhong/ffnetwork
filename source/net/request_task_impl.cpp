#include "request_task_impl.h"
#include "base/logging.h"
#include "utils/time_utils.h"
#include <net/response_impl.h>
#include <sstream>
#include <utility>

namespace ffnetwork {

RequestTaskImpl::RequestTaskImpl(
    std::shared_ptr<Request> request,
    const std::weak_ptr<RequestTaskInternalDelegate> &delegate)
    : request_(request), cancelled_(false), internal_delegate_(delegate),
      metrics_(std::make_shared<Metrics>()), identifier_(request->hash()) {
  handle_ = std::unique_ptr<HandleInfo>(new HandleInfo(request, this));
  response_ = std::make_shared<ResponseImpl>();
}

RequestTaskImpl::~RequestTaskImpl() = default;

std::string RequestTaskImpl::taskIdentifier() const { return identifier_; }

void RequestTaskImpl::Cancel() {
  if (auto delegate = internal_delegate_.lock()) {
    delegate->RequestTaskCancel(shared_from_this());
  }
}

bool RequestTaskImpl::isCancelled() { return cancelled_; }

void RequestTaskImpl::Resume() {
  metrics_->request_start_ms = NowTimeMillis();
  if (auto delegate = internal_delegate_.lock()) {
    delegate->RequestTaskStart(shared_from_this());
  }
}

CURL *RequestTaskImpl::handle() { return handle_->handle; }

void RequestTaskImpl::setCompletionCallback(
    CompletionCallback completionCallback) {
  completion_callback_ = std::move(completionCallback);
}

#pragma mark - HandleInfo

RequestTaskImpl::HandleInfo::HandleInfo(const std::shared_ptr<Request>& req,
    RequestTaskImpl* task_ptr) :
handle(nullptr),
request_hash(req->hash()),
request_headers(nullptr)
{
  handle = curl_easy_init();
  ConstructCurlHandle(req, task_ptr);
}

RequestTaskImpl::HandleInfo::~HandleInfo() {

  if (request_headers) {
    curl_slist_free_all(request_headers);
  }

  if (handle) {
    curl_easy_cleanup(handle);
  }
}

void RequestTaskImpl::HandleInfo::ConstructCurlHandle(const std::shared_ptr<Request>& request,
                                                      RequestTaskImpl* task_ptr) {

  curl_easy_setopt(handle, CURLOPT_URL, request->url().c_str());
  curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, task_ptr);
  curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_callback);
  curl_easy_setopt(handle, CURLOPT_WRITEHEADER, task_ptr);

  curl_easy_setopt(handle, CURLOPT_TIMEOUT, 30);

#if __APPLE__
  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, false);
#endif

  // Stash request hash in a pointer so we can get callback later
  curl_easy_setopt(handle, CURLOPT_PRIVATE, &request_hash);

  // Set method
  if (request->method() == GetMethod) {
    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
  } else if (request->method() == PutMethod) {
    curl_easy_setopt(handle, CURLOPT_PUT, 1);
  } else if (request->method() == PostMethod) {
    curl_easy_setopt(handle, CURLOPT_POST, 1);
  } else if (request->method() == HeadMethod) {
    curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
  } else {
    curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, request->method().c_str());
  }

  // Set data
  size_t data_length;
  const unsigned char *request_data = request->data(data_length);
  if (data_length) {
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, data_length);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, request_data);
  }

  // Set custom headers
  ConstructHeaders(request);
  curl_easy_setopt(handle, CURLOPT_HEADEROPT, CURLHEADER_UNIFIED);
  curl_easy_setopt(handle, CURLOPT_HTTPHEADER, request_headers);
}

void RequestTaskImpl::HandleInfo::ConstructHeaders(const std::shared_ptr<Request>& request) {
  struct curl_slist *headers = nullptr;
  for (auto const &header : request->headerMap()) {
    if (header.first == "Range") {
      curl_easy_setopt(handle, CURLOPT_RANGE, header.second.c_str());
      continue;
    }

    std::stringstream header_ss;
    header_ss << header.first << ": " << header.second;
    headers = curl_slist_append(headers, header_ss.str().c_str());
  }

  request_headers = headers;
}

#pragma mark - CurlCallback

size_t RequestTaskImpl::write_callback(char *data, size_t size, size_t nitems,
                                       void *str) {
  auto * task_ptr = static_cast<RequestTaskImpl *>(str);
  if (task_ptr == nullptr) {
    return 0;
  }
  task_ptr->OnReceiveData(data, size * nitems);
  return size * nitems;
}

size_t RequestTaskImpl::header_callback(char *data, size_t size, size_t nitems,
                                        void *str) {
  auto * task_ptr = static_cast<RequestTaskImpl *>(str);
  if (task_ptr == nullptr) {
    return 0;
  }

  task_ptr->OnReceiveHeader(data, size * nitems);
  return size * nitems;
}

void RequestTaskImpl::DidFinished(HttpStatusCode http_code,
                                  ResponseCode response_code) {
  auto request = request_;

  FF_LOG_P(DEBUG, "Got response for: %s", request->url().c_str());
  FF_LOG_P(DEBUG, "Response code: %lu", http_code);
  FillMetrics();

  response_->Construct(request, http_code, response_code, metrics_);

  if (completion_callback_) {
    completion_callback_(response_);
  }
}

void RequestTaskImpl::DidCancelled() {

  cancelled_ = true;

  std::shared_ptr<Response> cancelled_response = std::make_shared<ResponseImpl>(
      request_, HttpStatusCode::StatusCodeInvalid,
      ResponseCode::UserCancel, metrics_, true);

  if (completion_callback_) {
    completion_callback_(cancelled_response);
  }
}

void RequestTaskImpl::OnReceiveData(char *data, size_t length) {
  FF_LOG(INFO) << "receive data: " << length;
}

void RequestTaskImpl::OnReceiveHeader(char *data, size_t length) {
  std::string header(data, length), key, value;
  size_t pos;
  if ((pos = header.find(':')) != std::string::npos) {
    key = header.substr(0, pos);
    value = header.substr(std::min(pos + 2, header.length()));
  }

  if (!key.empty()) {
    response_->headerMap()[key] = value;
    FF_LOG(INFO) << "receive header: " << key << ": " << value;
  }
}

void RequestTaskImpl::FillMetrics() {

  if (!handle_->handle) {
    return;
  }

  auto handle = handle_->handle;
  auto metrics = metrics_;

  curl_easy_getinfo(handle, CURLINFO_NAMELOOKUP_TIME, &metrics->dns_time_ms);
  curl_easy_getinfo(handle, CURLINFO_CONNECT_TIME, &metrics->connect_time_ms);
  curl_easy_getinfo(handle, CURLINFO_APPCONNECT_TIME, &metrics->ssl_time_ms);
  curl_easy_getinfo(handle, CURLINFO_PRETRANSFER_TIME,
                    &metrics->pretransfer_time_ms);
  curl_easy_getinfo(handle, CURLINFO_STARTTRANSFER_TIME,
                    &metrics->transfer_start_time_ms);
  curl_easy_getinfo(handle, CURLINFO_TOTAL_TIME, &metrics->totoal_time_ms);

  metrics->dns_time_ms *= 1000;
  metrics->connect_time_ms *= 1000;
  metrics->ssl_time_ms *= 1000;
  metrics->pretransfer_time_ms *= 1000;
  metrics->transfer_start_time_ms *= 1000;
  metrics->totoal_time_ms *= 1000;

  curl_off_t upload_count, download_count;
  curl_easy_getinfo(handle, CURLINFO_SIZE_UPLOAD_T, &upload_count);
  curl_easy_getinfo(handle, CURLINFO_SIZE_DOWNLOAD_T, &download_count);

  metrics->send_byte_count = upload_count;
  metrics->receive_byte_count = download_count;

  metrics->request_end_ms = NowTimeMillis();
}

} // end of namespace ffnetwork
