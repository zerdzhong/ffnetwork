#ifndef FFNETWORK_MESSAGE_HANDLER_H
#define FFNETWORK_MESSAGE_HANDLER_H

namespace ffnetwork {

    struct Message;

    class MessageHandler {
    public:
        virtual ~MessageHandler();
        virtual void OnMessage(Message *msg) = 0;
    protected:
        MessageHandler() {}
    private:
        void operator=(const MessageHandler&) = delete;
        MessageHandler(const MessageHandler &) = delete;
    };

    template<class ReturnT, class FunctorT>
    class FunctorMessageHandler : public MessageHandler {
    public:
        explicit FunctorMessageHandler(const FunctorT& functor) : functor_(functor){
        }

        virtual void OnMessage(Message *msg) {
            result_ = functor_();
        }

        const ReturnT& result() {
            return result_;
        }

    private:
        FunctorT functor_;
        ReturnT result_;
    };

    template<class FunctorT>
    class FunctorMessageHandler<void, FunctorT> : public MessageHandler {
    public:
        explicit FunctorMessageHandler(const FunctorT& functor) : functor_(functor){
        }

        virtual void OnMessage(Message *msg) {
            functor_();
        }

        void result() const {}

    private:
        FunctorT functor_;
    };
}

#endif