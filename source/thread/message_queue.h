#ifndef FFNETWORK_MESSAGE_QUEUE_H
#define FFNETWORK_MESSAGE_QUEUE_H

#include "sigslot/sigslot.hpp"
#include "message_handler.h"
#include "critical_section.h"
#include <cstring>
#include <vector>
#include <list>
#include <cstdint>
#include <memory>

namespace ffnetwork {
    class MessageQueue;

    class MessageQueueManager {
    public:
        static void Add(MessageQueue* message_queue);
        static void Remove(MessageQueue* message_queue);
        static void Clear(MessageHandler *handler);

        static bool IsInitialized();
    private:
        static MessageQueueManager* Instance();
        MessageQueueManager();
        ~MessageQueueManager();

        void AddInternal(MessageQueue* message_quque);
        void RemoveInternal(MessageQueue* message_quque);
        void ClearInternal(MessageHandler *handler);

        static MessageQueueManager *instance_;
        std::vector<MessageQueue *> message_queues_;

        CriticalSection critical_section_;
    };

    class MessageData {
    public:
        MessageData() {}
        virtual ~MessageData() {}
    };

    template <class T>
    class TypedMessageData : public MessageData {
    public:
        explicit TypedMessageData(const T& data) : data_(data) {}
        const T& data() const { return data_; }
        T& data() { return data_; }
    private:
        T data_;
    };

    template <class T>
    class UniqueMessageData : public MessageData {
    public:
        explicit UniqueMessageData(std::unique_ptr<T> data)
                : data_(std::move(data)) {}

        const T& data() const { return *data_; }
        T& data() { return *data_; }
    private:
        std::unique_ptr<T> data_;
    };

    template <class T>
    class SharedMessageData : public MessageData {
    public:
        explicit SharedMessageData(std::shared_ptr<T> data) : data_(data) {}
        const std::shared_ptr<T>& data() const { return data_; }
        std::shared_ptr<T>& data() { return data_; }
    private:
        std::shared_ptr<T> data_;
    };


    const uint32_t MQID_ANY = static_cast<uint32_t>(-1);
    const uint32_t MQID_DISPOSE = static_cast<uint32_t>(-2);

    struct Message {
        Message() {
            memset(this, 0, sizeof(*this));
        }

        inline bool Match(MessageHandler* handler, uint32_t id) const {
            return (handler == NULL || handler == phandler)
                && (id == MQID_ANY || id == message_id);
        }

        MessageHandler* phandler;
        uint32_t message_id;
        MessageData* pdata;
        uint32_t ts_sensitive;
    };

    typedef std::list<Message> MessageList;

    class MessageQueue {
    public:
        static const int kForever = -1;

        MessageQueue();
        virtual ~MessageQueue();

        virtual void Quit();
        virtual bool IsQuitting();
        virtual void Restart();

        virtual bool Get(Message *pmsg, int cmsWait = kForever,
                   bool process_io = true);
        virtual bool Peek(Message *pmsg, int cmsWait = 0);
        virtual void Post(MessageHandler* phandler,
                        uint32_t id = 0,
                        MessageData* pdata = NULL,
                        bool time_sensitive = false);

        virtual void Clear(MessageHandler* phandler,
                            uint32_t id = MQID_ANY,
                            MessageList* removed = NULL);

        virtual void Dispatch(Message *pmsg);
        virtual void ReceiveSends();

        bool empty() const { return size() == 0u; }

        size_t size() const {
            CriticalScope cs(&critical_section_);  // msgq_.size() is not thread safe.
            return msg_queue_.size() + (fPeekKeep_ ? 1u : 0u);
        }

        // When this signal is sent out, any references to this queue should
        // no longer be used.
        sigslot::signal0<> SignalQueueDestroyed;

    protected:
        bool fStop_;
        bool fPeekKeep_;    
        Message msgPeek_;
        MessageList msg_queue_;
        mutable CriticalSection critical_section_;

    private:
        void operator=(const MessageQueue&) = delete;
        MessageQueue(const MessageQueue &) = delete;
    };

}

#endif
