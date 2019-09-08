//
// Created by zerdzhong on 2019-06-23.
//

#include "curl_client.h"

namespace ffnetwork {

    std::shared_ptr<Client> createCurlClient() {
        auto curl_client = std::make_shared<CurlClient>();
        return curl_client;
    }

    CurlClient::CurlClient() {
        curl_multi_handle_ = curl_multi_init();
        curl_multi_setopt(curl_multi_handle_, CURLMOPT_MAXCONNECTS, MAX_CONNECTIONS);
    }

    CurlClient::~CurlClient() {

    }

    std::shared_ptr<RequestTask> CurlClient::performRequest(const std::shared_ptr<Request> &request, std::function<void(
            const std::shared_ptr<Response> &)> callback) {
        return std::shared_ptr<RequestTask>();
    }

    void CurlClient::requestTaskDidCancel(const RequestTask *task) const {

    }

    std::shared_ptr<Response> CurlClient::performRequestSync(const std::shared_ptr<Request> &request) {
        return std::shared_ptr<Response>();
    }

#pragma mark HandleInfo
    CurlClient::HandleInfo::~HandleInfo() {

    }

    CurlClient::HandleInfo::HandleInfo(std::shared_ptr<Request> req,
                                       std::function<void(const std::shared_ptr<Response> &)> callback) {

    }

    CurlClient::HandleInfo::HandleInfo() {

    }
}
