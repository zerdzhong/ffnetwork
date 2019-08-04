#ifndef FFNETWORK_RESPONSE_H
#define FFNETWORK_RESPONSE_H

#include <string>
#include <memory>
#include <ffnetwork/request.h>

namespace ffnetwork {
    class Response {
    public:

        virtual const std::shared_ptr<Request> request() const = 0;
        virtual const unsigned char *data(size_t &data_length) const = 0;

        virtual const std::string& mimeType() const = 0;
        virtual const std::string& textEncodingName() const = 0;

        virtual long long expectedContentLength() const = 0;
        
        virtual unsigned int statusCode() const = 0;
        virtual bool cancelled() const = 0;
        
        virtual std::string operator[](const std::string &header_name) const = 0;
        virtual std::string &operator[](const std::string &header_name) = 0;

        virtual std::unordered_map<std::string, std::string> &headerMap() = 0;
        virtual std::unordered_map<std::string, std::string> headerMap() const = 0;

    };
}//end of namespace ffnetwork

#endif