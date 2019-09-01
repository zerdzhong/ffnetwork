//
// Created by zerdzhong on 2019-08-10.
//
#include "gtest/gtest.h"
#include "thread/critical_section.h"

#define private public
#define protected public

#include "thread/thread.h"

#undef private
#undef protected

#include "gunit.h"
#include "thread/async_invoker.h"
#include <functional>

using namespace ffnetwork;

TEST(ThreadTest, Names) {
// Default name
    Thread *thread;
    thread = new Thread();
    EXPECT_TRUE(thread->Start());
    thread->Stop();
    delete thread;

    thread = new Thread();
// Name with no object parameter
    EXPECT_TRUE(thread->SetName("No object", NULL));
    EXPECT_TRUE(thread->Start());
    thread->Stop();
    delete thread;

// long name
    thread = new Thread();
    EXPECT_TRUE(thread->SetName("Abcdefghijklmnopqrstuvwxyz1234567890", this));
    EXPECT_TRUE(thread->Start());
    thread->Stop();
    delete thread;
}

class CustomThread : public Thread {
public:
    CustomThread() {}
    virtual ~CustomThread() { Stop(); }
    bool Start() { return false; }
    bool WrapCurrent() {
        return Thread::WrapCurrent();
    }
    void UnwrapCurrent() {
        Thread::UnwrapCurrent();
    }
};

TEST(ThreadTest, Wrap) {
    Thread* current_thread = Thread::Current();
    current_thread->UnwrapCurrent();
    CustomThread* cthread = new CustomThread();
    EXPECT_TRUE(cthread->WrapCurrent());
    EXPECT_TRUE(cthread->running());
    EXPECT_FALSE(cthread->IsOwned());
    cthread->UnwrapCurrent();
    EXPECT_FALSE(cthread->running());
    delete cthread;
    current_thread->WrapCurrent();
}

class AtomicBool {
public:
    explicit AtomicBool(bool value = false) : flag_(value) {}

    AtomicBool &operator=(bool value) {
        CriticalScope scoped_lock(&cs_);
        flag_ = value;
        return *this;
    }

    bool get() const {
        CriticalScope scoped_lock(&cs_);
        return flag_;
    }

private:
    mutable CriticalSection cs_;
    bool flag_;
};

// Function objects to test Thread::Invoke.
struct FunctorA {
    int operator()() {
        return 41;
    }
};

class FunctorB {
public:
    explicit FunctorB(AtomicBool* flag) : flag_(flag) {}
    void operator()() { if (flag_) *flag_ = true; }
private:
    AtomicBool* flag_;
};

struct FunctorC {
    int operator()() {
        Thread::Current()->ProcessMessages(50);
        return 24;
    }
};

TEST(ThreadTest, Invoke) {
    // Create and start the thread.
    Thread thread;
    thread.Start();
    // Try calling functors.
    EXPECT_EQ(41, thread.Invoke<int>(FunctorA()));
    AtomicBool called;
    FunctorB f2(&called);
    thread.Invoke<void>(f2);
    EXPECT_TRUE(called.get());
    // Try calling bare functions.
    struct LocalFuncs {
        static int Func1() { return 999; }
        static void Func2() {}
    };
    EXPECT_EQ(999, thread.Invoke<int>(&LocalFuncs::Func1));
    thread.Invoke<void>(&LocalFuncs::Func2);
}

TEST(ThreadTest, TwoThreadsInvokeNoDeadlock) {
    AutoThread thread;
    Thread* current_thread = Thread::Current();
    ASSERT_TRUE(current_thread != NULL);
    Thread other_thread;
    other_thread.Start();
    struct LocalFuncs {
        static void Set(bool* out) { *out = true; }
        static void InvokeSet(Thread* thread, bool* out) {
            thread->Invoke<void>(std::bind(&Set, out));
        }
    };
    bool called = false;
    other_thread.Invoke<void>(
            std::bind(&LocalFuncs::InvokeSet, current_thread, &called));
    EXPECT_TRUE(called);
}

TEST(ThreadTest, ThreeThreadsInvoke) {
    AutoThread thread;
    Thread* thread_a = Thread::Current();
    Thread thread_b, thread_c;
    thread_b.Start();
    thread_c.Start();
    class LockedBool {
    public:
        explicit LockedBool(bool value) : value_(value) {}
        void Set(bool value) {
            CriticalScope lock(&crit_);
            value_ = value;
        }
        bool Get() {
            CriticalScope lock(&crit_);
            return value_;
        }
    private:
        CriticalSection crit_;
        bool value_ GUARDED_BY(crit_);
    };
    struct LocalFuncs {
        static void Set(LockedBool* out) { out->Set(true); }
        static void InvokeSet(Thread* thread, LockedBool* out) {
            thread->Invoke<void>(std::bind(&Set, out));
        }
        // Set |out| true and call InvokeSet on |thread|.
        static void SetAndInvokeSet(LockedBool* out,
                                    Thread* thread,
                                    LockedBool* out_inner) {
            out->Set(true);
            InvokeSet(thread, out_inner);
        }
        // Asynchronously invoke SetAndInvokeSet on |thread1| and wait until
        // |thread1| starts the call.
        static void AsyncInvokeSetAndWait(
                Thread* thread1, Thread* thread2, LockedBool* out) {
            CriticalSection crit;
            LockedBool async_invoked(false);
            AsyncInvoker invoker;
            invoker.AsyncInvoke<void>(
                    thread1, std::bind(&SetAndInvokeSet, &async_invoked, thread2, out));
            EXPECT_TRUE_WAIT(async_invoked.Get(), 2000);
        }
    };
    LockedBool thread_a_called(false);
    // Start the sequence A --(invoke)--> B --(async invoke)--> C --(invoke)--> A.
    // Thread B returns when C receives the call and C should be blocked until A
    // starts to process messages.
    thread_b.Invoke<void>(std::bind(&LocalFuncs::AsyncInvokeSetAndWait,
                               &thread_c, thread_a, &thread_a_called));
    EXPECT_FALSE(thread_a_called.Get());
    EXPECT_TRUE_WAIT(thread_a_called.Get(), 2000);
}

class AsyncInvokeTest : public testing::Test {
public:
    void IntCallback(int value) {
        EXPECT_EQ(expected_thread_, Thread::Current());
        int_value_ = value;
    }
    void AsyncInvokeIntCallback(AsyncInvoker* invoker, Thread* thread) {
        expected_thread_ = thread;
        invoker->AsyncInvoke(thread, FunctorC(),
                             &AsyncInvokeTest::IntCallback,
                             static_cast<AsyncInvokeTest*>(this));
        invoke_started_.Set();
    }
    void SetExpectedThreadForIntCallback(Thread* thread) {
        expected_thread_ = thread;
    }
protected:
    enum { kWaitTimeout = 1000 };
    AsyncInvokeTest()
            : int_value_(0),
              invoke_started_(true, false),
              expected_thread_(NULL) {}
    int int_value_;
    Event invoke_started_;
    Thread* expected_thread_;
};

TEST_F(AsyncInvokeTest, WithCallback) {
    AsyncInvoker invoker;
    // Create and start the thread.
    Thread thread;
    thread.Start();
    // Try calling functor.
    SetExpectedThreadForIntCallback(Thread::Current());
    invoker.AsyncInvoke(&thread, FunctorA(),
                        &AsyncInvokeTest::IntCallback,
                        static_cast<AsyncInvokeTest*>(this));
    EXPECT_EQ_WAIT(41, int_value_, kWaitTimeout);
}

TEST_F(AsyncInvokeTest, CancelInvoker) {
    // Create and start the thread.
    Thread thread;
    thread.Start();
    // Try destroying invoker during call.
    {
        AsyncInvoker invoker;
        invoker.AsyncInvoke(&thread, FunctorC(),
                            &AsyncInvokeTest::IntCallback,
                            static_cast<AsyncInvokeTest*>(this));
    }
    // With invoker gone, callback should be cancelled.
    Thread::Current()->ProcessMessages(kWaitTimeout);
    EXPECT_EQ(0, int_value_);
}

TEST_F(AsyncInvokeTest, CancelCallingThread) {
    AsyncInvoker invoker;
    { // Create and start the thread.
        Thread thread;
        thread.Start();
        // Try calling functor.
        thread.Invoke<void>(std::bind(&AsyncInvokeTest::AsyncInvokeIntCallback,
                                 static_cast<AsyncInvokeTest*>(this),
                                 &invoker, Thread::Current()));
        // Wait for the call to begin.
        ASSERT_TRUE(invoke_started_.Wait(kWaitTimeout));
    }
    // Calling thread is gone. Return message shouldn't happen.
    Thread::Current()->ProcessMessages(kWaitTimeout);
    EXPECT_EQ(0, int_value_);
}

TEST_F(AsyncInvokeTest, KillInvokerBeforeExecute) {
    Thread thread;
    thread.Start();
    {
        AsyncInvoker invoker;
        // Try calling functor.
        thread.Invoke<void>(std::bind(&AsyncInvokeTest::AsyncInvokeIntCallback,
                                 static_cast<AsyncInvokeTest*>(this),
                                 &invoker, Thread::Current()));
        // Wait for the call to begin.
        ASSERT_TRUE(invoke_started_.Wait(kWaitTimeout));
    }
    // Invoker is destroyed. Function should not execute.
    Thread::Current()->ProcessMessages(kWaitTimeout);
    EXPECT_EQ(0, int_value_);
}

class GuardedAsyncInvokeTest : public testing::Test {
public:
    void IntCallback(int value) {
        EXPECT_EQ(expected_thread_, Thread::Current());
        int_value_ = value;
    }
    void AsyncInvokeIntCallback(GuardedAsyncInvoker* invoker, Thread* thread) {
        expected_thread_ = thread;
        invoker->AsyncInvoke(FunctorC(), &GuardedAsyncInvokeTest::IntCallback,
                             static_cast<GuardedAsyncInvokeTest*>(this));
        invoke_started_.Set();
    }
    void SetExpectedThreadForIntCallback(Thread* thread) {
        expected_thread_ = thread;
    }
protected:
    const static int kWaitTimeout = 1000;
    GuardedAsyncInvokeTest()
    : int_value_(0),
    invoke_started_(true, false),
    expected_thread_(nullptr) {}
    int int_value_;
    Event invoke_started_;
    Thread* expected_thread_;
};

// Functor for creating an invoker.
struct CreateInvoker {
    CreateInvoker(std::shared_ptr<GuardedAsyncInvoker>* invoker) : invoker_(invoker) {}
    void operator()() { invoker_->reset(new GuardedAsyncInvoker()); }
    std::shared_ptr<GuardedAsyncInvoker>* invoker_;
};


// Test that we can call AsyncInvoke<void>() after the thread died.
TEST_F(GuardedAsyncInvokeTest, KillThreadFireAndForget) {
    // Create and start the thread.
    std::shared_ptr<Thread> thread(new Thread());
    thread->Start();
    std::shared_ptr<GuardedAsyncInvoker> invoker;
    // Create the invoker on |thread|.
    thread->Invoke<void>(CreateInvoker(&invoker));
    // Kill |thread|.
    thread = nullptr;
    // Try calling functor.
    AtomicBool called;
    EXPECT_FALSE(invoker->AsyncInvoke<void>(FunctorB(&called)));
    // With thread gone, nothing should happen.
    WAIT(called.get(), kWaitTimeout);
    EXPECT_FALSE(called.get());
}
// Test that we can call AsyncInvoke with callback after the thread died.
TEST_F(GuardedAsyncInvokeTest, KillThreadWithCallback) {
    // Create and start the thread.
    std::shared_ptr<Thread> thread(new Thread());
    thread->Start();
    std::shared_ptr<GuardedAsyncInvoker> invoker;
    // Create the invoker on |thread|.
    thread->Invoke<void>(CreateInvoker(&invoker));
    // Kill |thread|.
    thread = nullptr;
    // Try calling functor.
    EXPECT_FALSE(
            invoker->AsyncInvoke(FunctorC(), &GuardedAsyncInvokeTest::IntCallback,
                                 static_cast<GuardedAsyncInvokeTest*>(this)));
    // With thread gone, callback should be cancelled.
    Thread::Current()->ProcessMessages(kWaitTimeout);
    EXPECT_EQ(0, int_value_);
}
// The remaining tests check that GuardedAsyncInvoker behaves as AsyncInvoker
// when Thread is still alive.
TEST_F(GuardedAsyncInvokeTest, FireAndForget) {
    GuardedAsyncInvoker invoker;
    // Try calling functor.
    AtomicBool called;
    EXPECT_TRUE(invoker.AsyncInvoke<void>(FunctorB(&called)));
    EXPECT_TRUE_WAIT(called.get(), kWaitTimeout);
}
TEST_F(GuardedAsyncInvokeTest, WithCallback) {
    GuardedAsyncInvoker invoker;
    // Try calling functor.
    SetExpectedThreadForIntCallback(Thread::Current());
    EXPECT_TRUE(invoker.AsyncInvoke(FunctorA(),
                                    &GuardedAsyncInvokeTest::IntCallback,
                                    static_cast<GuardedAsyncInvokeTest*>(this)));
    EXPECT_EQ_WAIT(41, int_value_, kWaitTimeout);
}
TEST_F(GuardedAsyncInvokeTest, CancelInvoker) {
    // Try destroying invoker during call.
    {
        GuardedAsyncInvoker invoker;
        EXPECT_TRUE(
                invoker.AsyncInvoke(FunctorC(), &GuardedAsyncInvokeTest::IntCallback,
                                    static_cast<GuardedAsyncInvokeTest*>(this)));
    }
    // With invoker gone, callback should be cancelled.
    Thread::Current()->ProcessMessages(kWaitTimeout);
    EXPECT_EQ(0, int_value_);
}
TEST_F(GuardedAsyncInvokeTest, CancelCallingThread) {
    GuardedAsyncInvoker invoker;
    // Try destroying calling thread during call.
    {
        Thread thread;
        thread.Start();
        // Try calling functor.
        thread.Invoke<void>(std::bind(&GuardedAsyncInvokeTest::AsyncInvokeIntCallback,
                                 static_cast<GuardedAsyncInvokeTest*>(this),
                                 &invoker, Thread::Current()));
        // Wait for the call to begin.
        ASSERT_TRUE(invoke_started_.Wait(kWaitTimeout));
    }
    // Calling thread is gone. Return message shouldn't happen.
    Thread::Current()->ProcessMessages(kWaitTimeout);
    EXPECT_EQ(0, int_value_);
}
TEST_F(GuardedAsyncInvokeTest, KillInvokerBeforeExecute) {
    Thread thread;
    thread.Start();
    {
        GuardedAsyncInvoker invoker;
        // Try calling functor.
        thread.Invoke<void>(std::bind(&GuardedAsyncInvokeTest::AsyncInvokeIntCallback,
                                 static_cast<GuardedAsyncInvokeTest*>(this),
                                 &invoker, Thread::Current()));
        // Wait for the call to begin.
        ASSERT_TRUE(invoke_started_.Wait(kWaitTimeout));
    }
    // Invoker is destroyed. Function should not execute.
    Thread::Current()->ProcessMessages(kWaitTimeout);
    EXPECT_EQ(0, int_value_);
}
TEST_F(GuardedAsyncInvokeTest, Flush) {
    GuardedAsyncInvoker invoker;
    AtomicBool flag1;
    AtomicBool flag2;
    // Queue two async calls to the current thread.
    EXPECT_TRUE(invoker.AsyncInvoke<void>(FunctorB(&flag1)));
    EXPECT_TRUE(invoker.AsyncInvoke<void>(FunctorB(&flag2)));
    // Because we haven't pumped messages, these should not have run yet.
    EXPECT_FALSE(flag1.get());
    EXPECT_FALSE(flag2.get());
    // Force them to run now.
    EXPECT_TRUE(invoker.Flush());
    EXPECT_TRUE(flag1.get());
    EXPECT_TRUE(flag2.get());
}
TEST_F(GuardedAsyncInvokeTest, FlushWithIds) {
    GuardedAsyncInvoker invoker;
    AtomicBool flag1;
    AtomicBool flag2;
    // Queue two async calls to the current thread, one with a message id.
    EXPECT_TRUE(invoker.AsyncInvoke<void>(FunctorB(&flag1), 5));
    EXPECT_TRUE(invoker.AsyncInvoke<void>(FunctorB(&flag2)));
    // Because we haven't pumped messages, these should not have run yet.
    EXPECT_FALSE(flag1.get());
    EXPECT_FALSE(flag2.get());
    // Execute pending calls with id == 5.
    EXPECT_TRUE(invoker.Flush(5));
    EXPECT_TRUE(flag1.get());
    EXPECT_FALSE(flag2.get());
    flag1 = false;
    // Execute all pending calls. The id == 5 call should not execute again.
    EXPECT_TRUE(invoker.Flush());
    EXPECT_FALSE(flag1.get());
    EXPECT_TRUE(flag2.get());
}
