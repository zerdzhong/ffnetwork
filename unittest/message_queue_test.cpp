#include "gtest/gtest.h"

#define private public
#define protected public

#include "thread/message_queue.h"

#undef private
#undef protected

using namespace ffnetwork ;

class TestMessageQueue : public testing::Test, public MessageQueue {

}

struct DeletedLockChecker {
    DeletedLockChecker(MessageQueueTest* test, bool* was_locked, bool* deleted)
        : test(test), was_locked(was_locked), deleted(deleted) { }
    ~DeletedLockChecker() {
        *deleted = true;
        *was_locked = test->IsLocked();
    }
    MessageQueueTest* test;
    bool* was_locked;
    bool* deleted;
};