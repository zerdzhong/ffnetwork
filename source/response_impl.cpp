//
// Created by zerdzhong on 2019/9/9.
//

#include <ffnetwork/response_impl.h>

namespace ffnetwork {

    ResponseImpl::ResponseImpl(const std::shared_ptr<Request> &request, const unsigned char *data, size_t data_length,
                               StatusCode status_code, bool cancelled) :
    request_(request),
    data_(data_length == 0 ? nullptr : (unsigned char *)malloc(data_length)),
    data_length_(data_length),
    status_code_(status_code),
    cancelled_(cancelled) {
        if (data_length > 0) {
            memcpy(data_, data, data_length);
        }
    }

    ResponseImpl::ResponseImpl(const std::string &serialised, const unsigned char *data, size_t data_length,
                               const std::shared_ptr<Response> &response) :
            data_(data_length == 0 ? nullptr : (unsigned char *)malloc(data_length)),
            data_length_(data_length),
            status_code_(StatusCodeInvalid),
            cancelled_(false) {

    }

    ResponseImpl::~ResponseImpl() {

    }

    const std::shared_ptr<Request> ResponseImpl::request() const {
        return std::shared_ptr<Request>();
    }

    const unsigned char *ResponseImpl::data(size_t &data_length) const {
        return nullptr;
    }

    StatusCode ResponseImpl::statusCode() const {
        return StatusCodeTemporaryRedirect;
    }

    bool ResponseImpl::cancelled() const {
        return false;
    }

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

    void ResponseImpl::setMetadata(const std::string &key, const std::string &value) {

    }

    std::unordered_map<std::string, std::string> ResponseImpl::metadata() const {
        return std::unordered_map<std::string, std::string>();
    }

    std::string ResponseImpl::serialise() const {
        return std::string();
    }

}

