#include "gtest/gtest.h"

#define private public
#define protected public

#include "ffnetwork/request.h"

#undef private
#undef protected

#include <string>
#include <unordered_map>
#include "log/log_macro.h"

using namespace ffnetwork;

TEST(RequestTest, basic) {
    auto url = "https://github.com";
    std::unordered_map<std::string, std::string> header = {{"Range","0-"}, {"User-Agent","test_ua"}};
    auto request = createRequest(url, header);
    
    GTEST_ASSERT_EQ(request->url(), url);
    GTEST_ASSERT_EQ(request->headerMap(), header);

    auto request_dup = createRequest(request);
    GTEST_ASSERT_EQ(request_dup->url(), url);
    GTEST_ASSERT_EQ(request_dup->headerMap(), header);
}