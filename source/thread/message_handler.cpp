#include "message_handler.h"
#include "message_queue.h"

namespace ffnetwork {
    MessageHandler::~MessageHandler() {
        MessageQueueManager::Clear(this);
    }
}