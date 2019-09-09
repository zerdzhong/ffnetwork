//
// Created by zerdzhong on 2019/9/4.
//

#include "gtest/gtest.h"
#include "ffnetwork/client.h"

using namespace ffnetwork;

TEST(CurlClientTests, create) {
    auto client = createClient();
    EXPECT_NE((void *)0, client.get());
}

TEST(CurlClientTests, request) {
    auto client = createClient();
    EXPECT_NE((void *)0, client.get());

    auto url = "https://github.com";
    std::unordered_map<std::string, std::string> header = {{"Range","0-"}, {"User-Agent","test_ua"}};
    auto request = createRequest(url, header);

    auto response = client->performRequestSync(request);
    EXPECT_NE(response.get(), nullptr);
}

