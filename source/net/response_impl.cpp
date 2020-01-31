//
// Created by zerdzhong on 2019/9/9.
//

#include <cstring>
#include <net/response_impl.h>
#include <sstream>
#include <algorithm>
#include "url.h"

namespace ffnetwork {

ResponseImpl::ResponseImpl() : cancelled_(false) {}

ResponseImpl::ResponseImpl(const std::shared_ptr<Request> &request,
                           HttpStatusCode status_code,
                           ResponseCode response_code,
                           const std::shared_ptr<Metrics> &metrics,
                           bool cancelled)
    : request_(request), metrics_(metrics), status_code_(status_code),
      response_code_(response_code), cancelled_(cancelled) {}

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
  setUrl(request->url());
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

uint64_t ResponseImpl::expectedContentLength() const {
  return expected_content_length_;
}

std::string ResponseImpl::mimeType() const {
  return mime_type_;
}

std::string ResponseImpl::encodingType() const {
  return encoding_type_;
}

std::string ResponseImpl::suggestedFileName() const {
  return suggested_file_name_;
}

void ResponseImpl::UpdateHeader(const std::string &key,
                                const std::string &value) {

  headers_[key] = value;

  auto lower_key = key;
  std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
  if ("content-length" == lower_key ) {
    expected_content_length_ = std::strtoull(value.c_str(), nullptr, 0);
  } else if ("content-type" == lower_key) {
    mime_type_ = value;
    if (mime_type_.substr(mime_type_.length() - 2) == "\r\n" ) {
      mime_type_.erase(mime_type_.length() - 2);
    }
  } else if ("transfer-encoding" == lower_key) {
    encoding_type_ = value;
    if (encoding_type_.substr(encoding_type_.length() - 2) == "\r\n" ) {
      encoding_type_.erase(encoding_type_.length() - 2);
    }
  }
}
std::string ResponseImpl::url() const { return url_; }

void ResponseImpl::setUrl(const std::string &url) {
  url_ = url;
  auto path = Url(url_).path();
  auto last_slash_pos = path.find_last_of('/');
  if ( last_slash_pos != std::string::npos) {
    suggested_file_name_ = path.substr(last_slash_pos + 1);
  } else {
    suggested_file_name_ = path;
  }
}

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
      << "total_time_ms: " << metrics->totoal_time_ms << '\n'
      << "receive byte count: " << metrics->receive_byte_count << '\n'
      << "send byte count: " << metrics->send_byte_count << '\n'
      << "download speed "
      << metrics->receive_byte_count / 1024 / 1024 /
             (metrics->totoal_time_ms / 1000)
      << "MiB/s" << '\n';

  return iss.str();
}

} // namespace ffnetwork
