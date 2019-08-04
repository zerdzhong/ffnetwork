#include "message_queue.h"
#include <algorithm>
#include <sys/time.h>
#include <chrono>
#include <assert.h>

namespace ffnetwork {

#pragma mark MessageQueueManager

    const uint32_t kMaxMsgLatency = 150;  // 150 ms

    MessageQueueManager* MessageQueueManager::instance_ = NULL;
    
    MessageQueueManager* MessageQueueManager::Instance() {
    // Note: This is not thread safe, but it is first called before threads are
    // spawned.
        if (!instance_) {
            instance_ = new MessageQueueManager;
        }
        return instance_;
    }
    
    bool MessageQueueManager::IsInitialized() {
        return instance_ != NULL;
    }

    MessageQueueManager::MessageQueueManager() {
    }
    
    MessageQueueManager::~MessageQueueManager() {
    }

    void MessageQueueManager::Add(MessageQueue *message_queue) {
        return Instance()->AddInternal(message_queue);
    }
    
    void MessageQueueManager::AddInternal(MessageQueue *message_queue) {
        CriticalScope cs(&critical_section_);
        message_queues_.push_back(message_queue);
    }

    void MessageQueueManager::Remove(MessageQueue *message_queue) {
        if (!instance_) return;
        return Instance()->RemoveInternal(message_queue);
    }

    void MessageQueueManager::RemoveInternal(MessageQueue *message_queue) {
        bool destroy = false;
        {
            CriticalScope cs(&critical_section_);
            std::vector<MessageQueue *>::iterator iter;
            iter = std::find(message_queues_.begin(), message_queues_.end(),
                     message_queue);
            if (iter != message_queues_.end()) {
                message_queues_.erase(iter);
            }
            destroy = message_queues_.empty();
        }

        if (destroy) {
            instance_ = NULL;
            delete this;
        }
    }

    void MessageQueueManager::Clear(MessageHandler *handler) {
        if (!instance_) return;
        return Instance()->ClearInternal(handler);
    }

    void MessageQueueManager::ClearInternal(MessageHandler *handler) {
        CriticalScope cs(&critical_section_);
        for (auto iter = message_queues_.begin(); iter != message_queues_.end(); ++iter) {
            (*iter)->Clear(handler);
        }
    }

#pragma mark MessageQueue
    MessageQueue::MessageQueue() : fStop_(false), fPeekKeep_(false) {
        MessageQueueManager::Add(this);
    }

    MessageQueue::~MessageQueue() {
        MessageQueueManager::Remove(this);
        Clear(NULL);
    }

    void MessageQueue::Quit() {
        fStop_ = true;
    }
    bool MessageQueue::IsQuitting() {
        return fStop_;
    }
    void MessageQueue::Restart() {
        fStop_ = false;
    }

    bool MessageQueue::Peek(Message *pmsg, int cmsWait) {
        if (fPeekKeep_) {
            *pmsg = msgPeek_;
            return true;
        }
        if (!Get(pmsg, cmsWait)) {
            return false;
        }
        msgPeek_ = *pmsg;
        fPeekKeep_ = true;
        return true;
    }

    bool MessageQueue::Get(Message *pmsg, int cmsWait, bool process_io) {
        if (fPeekKeep_) {
            *pmsg = msgPeek_;
            fPeekKeep_ = false;
            return true;
        }

        int cmsTotal = cmsWait;
        int cmsElapsed = 0;
        uint64_t msStart = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        uint64_t msCurrent = msStart;

        while(true) {
            //Check 
            ReceiveSends();

            int cmsDelayNext = kForever;
            bool first_pass = true;

            while(true) {
                {
                    CriticalScope cs(&critical_section_);

                    if (msg_queue_.empty()) {
                        break;
                    } else {
                        *pmsg = msg_queue_.front();
                        msg_queue_.pop_front();
                    }
                }

                // If this was a dispose message, delete it and skip it.
                if (MQID_DISPOSE == pmsg->message_id) {
                    assert(NULL == pmsg->phandler);
                    delete pmsg->pdata;
                    *pmsg = Message();
                    continue;
                }

                return true;
            }

            if (fStop_) {
                break;
            }

            // Which is shorter, the delay wait or the asked wait?
            int cmsNext;
            if (cmsWait == kForever) {
                cmsNext = cmsDelayNext;
            } else {
                cmsNext = std::max(0, cmsTotal - cmsElapsed);
                if ((cmsDelayNext != kForever) && (cmsDelayNext < cmsNext))
                cmsNext = cmsDelayNext;
            }

            // If the specified timeout expired, return
            msCurrent = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            cmsElapsed = msCurrent - msStart;
            if (cmsWait != kForever) {
                if (cmsElapsed >= cmsWait)
                    return false;
            }
        }

        return false;
    }

    void MessageQueue::ReceiveSends() {
    }

    void MessageQueue::Post(MessageHandler* phandler,
                        uint32_t id,
                        MessageData* pdata,
                        bool time_sensitive) {
        if (fStop_) {
            return;
        }
        // Keep thread safe
        // Add the message to the end of the queue
        // Signal for the multiplexer to return
        CriticalScope cs(&critical_section_);
        Message msg;
        msg.phandler = phandler;
        msg.message_id = id;
        msg.pdata = pdata;
        if (time_sensitive) {
            uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            msg.ts_sensitive = now + kMaxMsgLatency;
        }
        msg_queue_.push_back(msg);
    }

}