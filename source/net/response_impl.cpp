//
// Created by zerdzhong on 2019/9/9.
//

#include <cstring>
#include <net/response_impl.h>
#include <sstream>

namespace ffnetwork {

ResponseImpl::ResponseImpl() : cancelled_(false) {}

ResponseImpl::ResponseImpl(const std::shared_ptr<Request> &request,
                           HttpStatusCode status_code,
                           ResponseCode response_code,
                           const std::shared_ptr<Metrics> &metrics,
                           bool cancelled)
    : request_(request), metrics_(metrics), status_code_(status_code),
      response_code_(response_code), cancelled_(cancelled) {
}

ResponseImpl::ResponseImpl(const std::string &serialised,
                           const unsigned char *data, size_t data_length,
                           const std::shared_ptr<Response> &response)
    : status_code_(HttpStatusCode::StatusCodeInvalid),
      response_code_(ResponseCode::OK), cancelled_(false) {}

ResponseImpl::~ResponseImpl() = default;

void ResponseImpl::Construct(const std::shared_ptr<Request> &request,
               HttpStatusCode status_code, ResponseCode response_code,
               const std::shared_ptr<Metrics> &metrics)
{
  request_ = request;
  status_code_ = status_code;
  response_code_ = response_code;
  metrics_ = metrics;
}

std::shared_ptr<Request> ResponseImpl::request() const {
  return request_;
}

std::shared_ptr<Metrics> ResponseImpl::metrics() const {
  return metrics_;
}

HttpStatusCode ResponseImpl::statusCode() const { return status_code_; }

ResponseCode ResponseImpl::responseCode() const { return response_code_; }

bool ResponseImpl::cancelled() const { return cancelled_; }

std::string ResponseImpl::operator[](const std::string &header_name) const {
  return headers_.at(header_name);
}

std::string &ResponseImpl::operator[](const std::string &header_name) {
  return headers_[header_name];
}

std::unordered_map<std::string, std::string> &ResponseImpl::headerMap() {
  return headers_;
}

std::unordered_map<std::string, std::string> ResponseImpl::headerMap() const {
  return headers_;
}

void ResponseImpl::setMetadata(const std::string &key,
                               const std::string &value) {}

std::unordered_map<std::string, std::string> ResponseImpl::metadata() const {
  return std::unordered_map<std::string, std::string>();
}

std::string ResponseImpl::serialise() const { return std::string(); }
uint64_t ResponseImpl::expectedContentLength() const { return 0; }

std::string metrics_dump_info(Metrics *metrics) {
  std::stringstream iss;

  iss << '\n'
      << "request_start_time: " << metrics->request_start_ms << '\n'
      << "request_end_ms: " << metrics->request_end_ms << '\n'
      << "dns_time_ms: " << metrics->dns_time_ms << '\n'
      << "connect_time_ms: " << metrics->connect_time_ms << '\n'
      << "ssl_time_ms: " << metrics->ssl_time_ms << '\n'
      << "pretransfer_time_ms: " << metrics->pretransfer_time_ms << '\n'
      << "transfer_start_time_ms: " << metrics->transfer_start_time_ms << '\n'
      << "totoal_time_ms: " << metrics->totoal_time_ms << '\n'
      << "receive byte count: " << metrics->receive_byte_count << '\n'
      << "send byte count: " << metrics->send_byte_count << '\n';

  return iss.str();
}

} // namespace ffnetwork
