#ifndef FFNETWORK_CLIENT_H
#define FFNETWORK_CLIENT_H

#include <string>
#include <memory>
#include <functional>
#include <ffnetwork/Request.h>
#include <ffnetwork/Response.h>
#include <ffnetwork/RequestTask.h>


namespace ffnetwork {
    class Client {
        public:
        virtual ~Client();

        virtual std::shared_ptr<RequestTask> performRequest(
            const std::shared_ptr<Request> &request,
            std::function<void(const std::shared_ptr<Response> &)> callback) = 0;

        virtual std::shared_ptr<Response> performRequestSync(const std::shared_ptr<Request> &request) = 0;
    };

    extern std::shared_ptr<Client> createClient();
}// namespace ffnetwork

#endif