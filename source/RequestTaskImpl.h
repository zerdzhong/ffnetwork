#include <ffnetwork/RequestTask.h>
#include <ffnetwork/RequestTaskDelegate.h>

namespace ffnetowork {
    class RequestTaskImpl : public RequestTask {
    public:

        RequestTaskImpl(const std::weak_ptr<RequestTaskDelegate> &delegate,
                             const std::string &identifier);

        virtual ~RequestTaskImpl();

        std::string taskIdentifier() override;
        void cancel() override;
        bool cancelled() override;

    private:
        bool cancelled_;
        const std::string identifier_;
        const std::weak_ptr<RequestTaskDelegate> delegate_;
    };
}//end of namespace ffnetwork