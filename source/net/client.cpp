#include <ffnetwork/client.h>
#include <atomic>
#include "curl_client.h"

namespace ffnetwork {
    Client::~Client() {

    }

    std::shared_ptr<Response> Client::PerformRequestSync(const std::shared_ptr<Request> &request) {
        std::mutex mutex;
        std::condition_variable cv;
        std::atomic<bool> response_ready(false);
        std::shared_ptr<Response> output_response = nullptr;
        PerformRequest(request, [&](const std::shared_ptr<Response> &response) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                output_response = response;
                response_ready = true;
            }
            cv.notify_one();
        });
        std::unique_lock<std::mutex> lock(mutex);
        while (!response_ready) {
            cv.wait(lock);
        }
        return output_response;
    }

    std::shared_ptr<Client> CreateClient() {
        return CreateCurlClient();
    }

}
