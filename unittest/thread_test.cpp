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
