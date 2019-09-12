#include "gtest/gtest.h"

#define private public
#define protected public

#include "thread/message_queue.h"

#undef private
#undef protected

#include "thread/thread.h"
#include "log/log_macro.h"
#include "utils/time_utils.h"

using namespace ffnetwork ;

class MessageQueueTest : public testing::Test, public MessageQueue {
public:
    bool IsLocked() {
        return false;
    }
};

TEST_F(MessageQueueTest, DelayedPosts) {
    MessageQueue q;
    uint64_t now = NowTimeMicros();
    q.PostAt(now, NULL, 3);
    q.PostAt(now - 2, NULL, 0);
    q.PostAt(now - 1, NULL, 1);
    q.PostAt(now, NULL, 4);
    q.PostAt(now - 1, NULL, 2);
    Message msg;
    for (size_t i=0; i<5; ++i) {
        memset(&msg, 0, sizeof(msg));
        EXPECT_TRUE(q.Get(&msg, 0));
        EXPECT_EQ(i, msg.message_id);
    }
    EXPECT_FALSE(q.Get(&msg, 0));  // No more messages
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

TEST_F(MessageQueueTest, DisposeNotLocked) {
    bool was_locked = true;
    bool deleted = false;
    DeletedLockChecker* d = new DeletedLockChecker(this, &was_locked, &deleted);
    Dispose(d);
    Message msg;
    EXPECT_FALSE(Get(&msg, 0));
    EXPECT_TRUE(deleted);
    EXPECT_FALSE(was_locked);
}

class DeletedMessageHandler : public MessageHandler {
public:
    explicit DeletedMessageHandler(bool* deleted) : deleted_(deleted) { }
    ~DeletedMessageHandler() {
        *deleted_ = true;
    }
    void OnMessage(Message* msg) { }
private:
    bool* deleted_;
};

TEST_F(MessageQueueTest, DiposeHandlerWithPostedMessagePending) {
    bool deleted = false;
    DeletedMessageHandler *handler = new DeletedMessageHandler(&deleted);
    // First, post a dispose.
    Dispose(handler);
    // Now, post a message, which should *not* be returned by Get().
    Post(handler, 1);
    Message msg;
    EXPECT_FALSE(Get(&msg, 0));
    EXPECT_TRUE(deleted);
}

struct UnwrapMainThreadScope {
    UnwrapMainThreadScope() : rewrap_(Thread::Current() != NULL) {
        if (rewrap_) ThreadManager::Instance()->UnwrapCurrentThread();
    }
    ~UnwrapMainThreadScope() {
        if (rewrap_) ThreadManager::Instance()->WrapCurrentThread();
    }
private:
    bool rewrap_;
};
TEST(MessageQueueManager, Clear) {
    UnwrapMainThreadScope s;
    if (MessageQueueManager::IsInitialized()) {
        LOGD( "Unable to run MessageQueueManager::Clear test, since the \n"
              "MessageQueueManager was already initialized by some \n"
              "other test in this run.");
        return;
    }
    bool deleted = false;
    DeletedMessageHandler* handler = new DeletedMessageHandler(&deleted);
    delete handler;
    EXPECT_TRUE(deleted);
    EXPECT_FALSE(MessageQueueManager::IsInitialized());
}
