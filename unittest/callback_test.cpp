#include "gtest/gtest.h"
#include "thread/callback.h"
#include <functional>

using namespace ffnetwork;

namespace {
    void f() {}
    int g() {return 42;}
    int h(int x) {return x * x;}
    void i(int &x) {x *= x;}
    struct BindTester {
        int a() {return 24;}
        int b(int x) const {return x * x;}
    };
}//namespace

TEST(CallbackTest, VoidReturn) {
    Callback0<void> cb;
    EXPECT_TRUE(cb.empty());
    cb(); // Executing an empty callback should not crash.
    cb = Callback0<void>(&f);
    EXPECT_FALSE(cb.empty());
    cb();
}

TEST(CallbackTest, IntReturn) {
    Callback0<int> cb;
    EXPECT_TRUE(cb.empty());
    cb = Callback0<int>(&g);
    EXPECT_FALSE(cb.empty());
    EXPECT_EQ(42, cb());
    EXPECT_EQ(42, cb());
}

TEST(CallbackTest, OneParam) {
    Callback1<int, int> cb1(&h);
    EXPECT_FALSE(cb1.empty());
    EXPECT_EQ(9, cb1(-3));
    EXPECT_EQ(100, cb1(10));
    // Try clearing a callback.
    cb1 = Callback1<int, int>();
    EXPECT_TRUE(cb1.empty());

    // Try a callback with a ref parameter.
    Callback1<void, int&> cb2(&i);
    int x = 3;
    cb2(x);
    EXPECT_EQ(9, x);
    cb2(x);
    EXPECT_EQ(81, x);
}

TEST(CallbackTest, WithBind) {
    BindTester t;
    Callback0<int> cb1 = std::bind(&BindTester::a, &t);
    EXPECT_EQ(24, cb1());
    EXPECT_EQ(24, cb1());
    cb1 = std::bind(&BindTester::b, &t, 6);
    EXPECT_EQ(36, cb1());
    EXPECT_EQ(36, cb1());
    cb1 = std::bind(&BindTester::b, &t, 5);
    EXPECT_EQ(25, cb1());
    EXPECT_EQ(25, cb1());
}