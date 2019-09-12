#ifndef FFNETWORK_CLIENT_H
#define FFNETWORK_CLIENT_H

#include <string>
#include <memory>
#include <functional>
#include <ffnetwork/request.h>
#include <ffnetwork/response.h>
#include <ffnetwork/request_task.h>


namespace ffnetwork {
    class Client {
        public:
        virtual ~Client();

        virtual std::shared_ptr<RequestTask> PerformRequest(
            const std::shared_ptr<Request> &request,
            std::function<void(const std::shared_ptr<Response> &)> callback) = 0;

        virtual std::shared_ptr<Response> PerformRequestSync(const std::shared_ptr<Request> &request);
    };

    extern std::shared_ptr<Client> CreateClient();
}// namespace ffnetwork

#endif
