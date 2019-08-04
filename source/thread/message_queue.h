#ifndef FFNETWORK_MESSAGE_QUEUE_H
#define FFNETWORK_MESSAGE_QUEUE_H

#include "message_handler.h"
#include "critical_section.h"
#include <string.h>
#include <vector>
#include <list>
#include <stdint.h>

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

    const uint32_t MQID_ANY = static_cast<uint32_t>(-1);
    const uint32_t MQID_DISPOSE = static_cast<uint32_t>(-2);

    struct Message {
        Message() {
            memset(this, 0, sizeof(*this));
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