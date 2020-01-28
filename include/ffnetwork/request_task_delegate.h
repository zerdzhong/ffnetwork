#ifndef FFNETWORK_REQUEST_TASK_DELEGATE_H
#define FFNETWORK_REQUEST_TASK_DELEGATE_H


namespace ffnetwork {
	class RequestTask;

    class RequestTaskDelegate {
    public:
        virtual void RequestTaskCancel(const std::shared_ptr<RequestTask> &task) = 0;
        virtual void RequestTaskStart(const std::shared_ptr<RequestTask> &task) = 0;
    };
}//end of namespace ffnetwork

#endif