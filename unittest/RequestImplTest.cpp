#include "gtest/gtest.h"
#include <memory>

#define private public
#define protected public

#include "RequestImpl.h"

#undef private
#undef protected


using namespace ffnetwork;

 TEST(RequestImplTest, basic) {
     auto request_impl = new RequestImpl("https://github.com",{{"Range","0-"}});
     EXPECT_EQ(request_impl->url(), "https://github.com");
 }