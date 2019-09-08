//
// Created by zerdzhong on 2019-06-23.
//

#ifndef FFNETWORK_CURL_CLIENT_H
#define FFNETWORK_CURL_CLIENT_H

#include <memory>
#include <unordered_map>
#include <ffnetwork/client.h>
#include <ffnetwork/request_task_delegate.h>
#include "curl/curl.h"

namespace ffnetwork {
    class CurlClient : public Client,
            public RequestTaskDelegate
            {

        struct HandleInfo {
            CURL *handle;
            const std::shared_ptr<Request> request;
            std::string request_hash;
            std::string response;
            curl_slist *request_headers;
            std::unordered_map<std::string, std::string> response_headers;
            std::function<void(const std::shared_ptr<Response> &)> callback;

            HandleInfo(std::shared_ptr<Request> req,
                       std::function<void(const std::shared_ptr<Response> &)> callback);
            HandleInfo();
            ~HandleInfo();

            void configureHeaders();
            void configureCurlHandle();
        };

    public:
        CurlClient();
        ~CurlClient() override;

        static const long MAX_CONNECTIONS = 10;

        // Client
        std::shared_ptr<RequestTask> performRequest(
                const std::shared_ptr<Request> &request,
                std::function<void(const std::shared_ptr<Response> &)> callback) override;

        std::shared_ptr<Response> performRequestSync(const std::shared_ptr<Request> &request) override ;

        // RequestTokenDelegate
        void requestTaskDidCancel(const RequestTask* task) const override;

    private:
        CURLM *curl_multi_handle_;
        std::unordered_map<std::string, std::unique_ptr<HandleInfo>> _handles;

    // Curl callbacks
    public:
        static size_t write_callback(char *data, size_t size, size_t nitems, void *str);
        static size_t header_callback(char *data, size_t size, size_t nitems, void *str);
    };

    extern std::shared_ptr<Client> createCurlClient();
}


#endif //FFNETWORK_CURLCLIENT_H
