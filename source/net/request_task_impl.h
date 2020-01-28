#ifndef FFNETWORK_REQUEST_TASK_IMPL_H
#define FFNETWORK_REQUEST_TASK_IMPL_H

#include <ffnetwork/request_task.h>
#include <ffnetwork/request_task_delegate.h>
#include "curl/curl.h"
#include <ffnetwork/client.h>
#include <memory>

namespace ffnetwork {

class RequestTaskImpl : public RequestTask, public std::enable_shared_from_this<RequestTaskImpl> {

	struct HandleInfo {
		CURL *handle;

		const std::shared_ptr<Request> request;
		std::string request_hash;
		curl_slist *request_headers;

		std::string response;
		std::unordered_map<std::string, std::string> response_headers;

		explicit HandleInfo(std::shared_ptr<Request> req);
		~HandleInfo();

		void ConfigureHeaders();
		void ConfigureCurlHandle();
	};

    public:
        RequestTaskImpl(const std::shared_ptr<Request> request, const std::weak_ptr<RequestTaskDelegate> &delegate);
        virtual ~RequestTaskImpl();

        std::string taskIdentifier() const override;
		void setDelegate(std::weak_ptr<RequestTaskDelegate> delegate) override ;
		void setCompletionCallback(CompletionCallback completionCallback);

		void resume() override;

        void cancel() override;
        bool cancelled() override;

        void didCancel();
        void didFinished(HttpStatusCode http_code, ResponseCode response_code);

		CURL* handle();

	private:
		void fillMetrics();

    private:
        bool cancelled_;
        std::weak_ptr<RequestTaskDelegate> delegate_;
        std::unique_ptr<HandleInfo> handle_;

        const std::string identifier_;

		const std::shared_ptr<Metrics> metrics_;
		const std::shared_ptr<Request> request_;

        CompletionCallback completion_callback_;

	// Curl callbacks
	public:
		static size_t write_callback(char *data, size_t size, size_t nitems, void *str);
		static size_t header_callback(char *data, size_t size, size_t nitems, void *str);
    };
}//end of namespace ffnetwork

#endif
