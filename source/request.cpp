#include <ffnetwork/request.h>
#include "request_impl.h"

namespace ffnetwork {
    const std::string GetMethod = "GET";
    const std::string PostMethod = "POST";
    const std::string PutMethod = "PUT";
    const std::string HeadMethod = "HEAD";

    std::shared_ptr<Request> createRequest(const std::string &url, const std::unordered_map<std::string, std::string> &header_map) {
        return std::make_shared<RequestImpl>(url, header_map);
    }

    std::shared_ptr<Request> createRequest(const std::shared_ptr<Request> &request) {
        return std::make_shared<RequestImpl>(*request.get());
    }
}