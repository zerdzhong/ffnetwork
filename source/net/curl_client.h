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
#include "base/thread/thread.h"
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace ffnetwork {

    class CurlClient : public Client,
            public RequestTaskDelegate,
public std::enable_shared_from_this<CurlClient>
            {

        struct HandleInfo {
            CURL *handle;
            const std::shared_ptr<Request> request;
            std::string request_hash;
            std::string response;
            curl_slist *request_headers;
            std::unordered_map<std::string, std::string> response_headers;
            std::function<void(const std::shared_ptr<Response> &)> callback;
            const std::shared_ptr<Metrics> metrics;

            HandleInfo(const std::shared_ptr<Request>& req,
                       const std::shared_ptr<Metrics>& metrics,
                       std::function<void(const std::shared_ptr<Response> &)> callback);
            HandleInfo();
            ~HandleInfo();

            void ConfigureHeaders();
            void ConfigureCurlHandle();
        };

    public:
        CurlClient();
        ~CurlClient() override;

        static const long MAX_CONNECTIONS = 10;

        // Client
        std::shared_ptr<RequestTask> PerformRequest(
                const std::shared_ptr<Request> &request,
                std::function<void(const std::shared_ptr<Response> &)> callback) override;

        void CancelRequest(const std::string& hash);

        // RequestTokenDelegate
        void RequestTaskDidCancel(const std::shared_ptr<RequestTask> &task) override;

        // Runnable
        void Run();

    private:
        CURLM *curl_multi_handle_;
        std::unordered_map<std::string, std::unique_ptr<HandleInfo>> handles_;

        std::mutex client_mutex_;
        std::condition_variable new_req_condition_;
        bool have_new_request_;
        std::atomic<bool> is_terminated_;
        ffbase::Thread request_thread_;

        bool use_multi_wait_;

        void CleanupRequest(const std::string& hash);

        void WaitMulti(long timeout_ms);
        void WaitFD(long timeout_ms);
        bool HandleCurlMsg();
        static ResponseCode ConvertCurlCode(CURLcode);
        static void ConfigMetrics(Metrics* metrics, CURL *handle);

    // Curl callbacks
    public:
        static size_t write_callback(char *data, size_t size, size_t nitems, void *str);
        static size_t header_callback(char *data, size_t size, size_t nitems, void *str);
    };

    extern std::shared_ptr<Client> CreateCurlClient();
}


#endif //FFNETWORK_CURLCLIENT_H
