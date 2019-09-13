#ifndef FFNETWORK_RESPONSE_H
#define FFNETWORK_RESPONSE_H

#include <string>
#include <memory>
#include <ffnetwork/request.h>

namespace ffnetwork {

    enum class HttpStatusCode : int {
        StatusCodeInvalid = 0,
        // Informational
        StatusCodeContinue = 100,
        StatusCodeSwitchProtocols = 101,
        // Successful
        StatusCodeOK = 200,
        StatusCodeCreated = 201,
        StatusCodeAccepted = 202,
        StatusCodeNonAuthoritativeInformation = 203,
        StatusCodeNoContent = 204,
        StatusCodeResetContent = 205,
        StatusCodePartialContent = 206,
        // Redirection
        StatusCodeMovedMultipleChoices = 300,
        StatusCodeMovedPermanently = 301,
        StatusCodeFound = 302,
        StatusCodeSeeOther = 303,
        StatusCodeNotModified = 304,
        StatusCodeUseProxy = 305,
        StatusCodeUnused = 306,
        StatusCodeTemporaryRedirect = 307,
        // Client Error
        StatusCodeBadRequest = 400,
        StatusCodeUnauthorised = 401,
        StatusCodePaymentRequired = 402,
        StatusCodeForbidden = 403,
        StatusCodeNotFound = 404,
        StatusCodeMethodNotAllowed = 405,
        StatusCodeNotAcceptable = 406,
        StatusCodeProxyAuthenticationRequired = 407,
        StatusCodeRequestTimeout = 408,
        StatusCodeConflict = 409,
        StatusCodeGone = 410,
        StatusCodeLengthRequired = 411,
        StatusCodePreconditionFailed = 412,
        StatusCodeRequestEntityTooLarge = 413,
        StatusCodeRequestURITooLong = 414,
        StatusCodeUnsupportedMediaTypes = 415,
        StatusCodeRequestRangeUnsatisfied = 416,
        StatusCodeExpectationFail = 417,
        // Server Error
        StatusCodeInternalServerError = 500,
        StatusCodeNotImplemented = 501,
        StatusCodeBadGateway = 502,
        StatusCodeServiceUnavailable = 503,
        StatusCodeGatewayTimeout = 504,
        StatusCodeHTTPVersionNotSupported = 505
    };
        
    enum class ResponseCode : int {
        Invalid             = -1,
        OK                  = 0,    //success
        InvalidHandle       = 1000, // invalid handle
        InvalidParameter,           //invalid parameter
        RuntimeError,               //runtime error
        Timeout,                    //timeout
        UserCancel,                 //user canceld
        ConnectToServerFailed,      //network connection failed
        InvalidUrl,                 //invalid url format
        UnknownError,
    };

    class Response {
    public:

        virtual const std::shared_ptr<Request> request() const = 0;
        virtual const unsigned char *data(size_t &data_length) const = 0;
        virtual HttpStatusCode statusCode() const = 0;
        virtual ResponseCode responseCode() const = 0;
        virtual bool cancelled() const = 0;
        virtual std::string serialise() const = 0;
        virtual std::string operator[](const std::string &header_name) const = 0;
        virtual std::string &operator[](const std::string &header_name) = 0;
        virtual std::unordered_map<std::string, std::string> &headerMap() = 0;
        virtual std::unordered_map<std::string, std::string> headerMap() const = 0;
        virtual std::unordered_map<std::string, std::string> metadata() const = 0;
        virtual void setMetadata(const std::string &key, const std::string &value) = 0;

    };
}//end of namespace ffnetwork

#endif
