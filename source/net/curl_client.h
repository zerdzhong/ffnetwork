//
// Created by zerdzhong on 2019-06-23.
//

#ifndef FFNETWORK_CURL_CLIENT_H
#define FFNETWORK_CURL_CLIENT_H

#include <memory>
#include <unordered_map>
#include <ffnetwork/client.h>
#include "request_task_impl.h"
#include "curl/curl.h"
#include "base/thread/thread.h"
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace ffnetwork {

    class CurlClient : public Client, public std::enable_shared_from_this<CurlClient>, public RequestTaskDelegate {

    public:
        CurlClient();
        ~CurlClient() override;

        static const long MAX_CONNECTIONS = 10;

        // Client
        std::shared_ptr<RequestTask> PerformRequest(
                const std::shared_ptr<Request> &request,
                std::function<void(const std::shared_ptr<Response> &)> callback) override;

		std::shared_ptr<RequestTask> TaskWithRequest(const std::shared_ptr<Request> &request, CompletionCallback) override ;

        // Runnable
        void Run();

    private:
        CURLM *curl_multi_handle_;
        std::unordered_map<std::string, std::shared_ptr<RequestTaskImpl>> handles_;

        std::mutex client_mutex_;
        std::condition_variable req_condition_;
        bool have_new_request_;
        std::atomic<bool> is_terminated_;
        ffbase::Thread request_thread_;

        bool use_multi_wait_;

		//RequestTaskDelegate
		void RequestTaskCancel(const std::shared_ptr<RequestTask> &task) override ;
		void RequestTaskStart(const std::shared_ptr<RequestTask> &task) override ;

		void StartRequest(const std::string& hash);
		void CancelRequest(const std::string& hash);
        void CleanupRequest(const std::string& hash);

        void WaitMulti(long timeout_ms);
        void WaitFD(long timeout_ms);
        bool HandleCurlMsg();
        static ResponseCode ConvertCurlCode(CURLcode);
    };

    extern std::shared_ptr<Client> CreateCurlClient();
}


#endif //FFNETWORK_CURLCLIENT_H
