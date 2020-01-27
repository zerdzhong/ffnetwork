#include "request_impl.h"
#include <cstring>
#include <vector>
#include <sstream>
#include <algorithm>

#include "utils/sha256.h"

namespace ffnetwork {
    RequestImpl::RequestImpl(const std::string& url,
        const std::unordered_map<std::string, std::string>& header_map):
    url_(url), headers_(header_map), method_(GetMethod), data_(nullptr), data_length_(0)
    {

    }
    
    RequestImpl::RequestImpl(const Request &request) :
    url_(request.url()), headers_(request.headerMap()), method_(request.method()), data_(nullptr), data_length_(0)     
    {
        size_t data_length = 0;
        const unsigned char *data = request.data(data_length);
        if (data_length > 0) {
            data_ = (unsigned char *) malloc(data_length);
            memcpy(data_, data, data_length);
            data_length_ = data_length;
        }
    }
    RequestImpl::~RequestImpl() {
        if (data_) {
            free(data_);
            data_ = nullptr;
            data_length_ = 0;
        }
    }

    std::string RequestImpl::url() const {
        return url_;
    }

    void RequestImpl::setUrl(const std::string &url){
        url_ = url;
    }
        
    std::unordered_map<std::string, std::string>& RequestImpl::headerMap() {
        return headers_;
    }
    
    std::unordered_map<std::string, std::string> RequestImpl::headerMap() const {
        return headers_;
    }

    std::string RequestImpl::method() const {
        return method_;
    }
    
    void RequestImpl::setMethod(const std::string &method) {
        method_ = method;
    };

    const unsigned char* RequestImpl::data(size_t &data_length) const {
        data_length = data_length_;
        return data_;
    }
    
    void RequestImpl::setData(const unsigned char *data, size_t data_length) {
        data_length_ = data_length;
        if (data_) {
            free(data_);
            data_ = nullptr;
            data_length_ = 0;
        }
        
        if (data_length > 0) {
            data_ = (unsigned char *)malloc(data_length + 1);
            memcpy(data_, data, data_length);
            data_length_ = data_length;
            data_[data_length] = 0;
        }
    }

    std::string RequestImpl::hash() const {
        // Support "Vary" headers
        std::vector<std::string> excluded_headers;
        const auto &vary_iterator = headers_.find("Vary");
        if (vary_iterator != headers_.end()) {
            std::istringstream ss((*vary_iterator).second);
            std::string token;
            while (std::getline(ss, token, ',')) {
                token.erase(remove_if(token.begin(), token.end(), isspace), token.end());
                excluded_headers.push_back(token);
            }
        }

        std::string amalgamation = url();
        for (const auto &header_pair : headers_) {
            if (std::find(excluded_headers.begin(), excluded_headers.end(), header_pair.first) !=
                excluded_headers.end()) {
                continue;
            }
            amalgamation += header_pair.first + header_pair.second;
        }
        if (data_ != nullptr) {
            amalgamation.append((const char *)data_, data_length_);
        }
        return sha256(amalgamation);
    }
        
    std::string RequestImpl::serialise() const {
        return "";
    }
}
