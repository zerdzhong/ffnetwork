#include "gtest/gtest.h"
#include <memory>
#include <unordered_map>
#define private public
#define protected public

#include "net/request_impl.h"

#undef private
#undef protected


using namespace ffnetwork;

 TEST(RequestImplTest, basic) {
     std::unordered_map<std::string, std::string> headers = {
             {"Range","0-"},
             {"User-Agent", "test-ua"}
             };
     std::string url = "https://github.com";
     auto request_impl = new RequestImpl(url, headers);
     GTEST_ASSERT_EQ(request_impl->url(), url);
     GTEST_ASSERT_EQ(request_impl->headerMap(), headers);
 }