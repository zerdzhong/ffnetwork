//
// Created by zerdzhong on 2019/9/4.
//

#include "gtest/gtest.h"
#include "ffnetwork/client.h"
#include "log/log_macro.h"
#include <atomic>
#include <mutex>
#include <condition_variable>

using namespace ffnetwork;

TEST(CurlClientTests, create) {
    auto client = CreateClient();
    EXPECT_NE((void *)0, client.get());
}

TEST(CurlClientTests, request) {
    auto client = CreateClient();
    EXPECT_NE((void *)0, client.get());

    auto url = "https://github.com";
    std::unordered_map<std::string, std::string> header = {{"Range","0-"}, {"User-Agent","test_ua"}};
    auto request = CreateRequest(url, header);

    auto response = client->PerformRequestSync(request);
    EXPECT_NE(response.get(), nullptr);
    EXPECT_EQ(response->request()->headerMap().size(), 2);
    EXPECT_EQ(response->request()->headerMap()["Range"], "0-");
    EXPECT_EQ(response->request()->headerMap()["User-Agent"], "test_ua");
}

TEST(CurlClientTests, request_batch) {
    auto client = CreateClient();
    EXPECT_NE((void *)0, client.get());

    std::unordered_map<std::string, std::string> header = {{"Range","0-"}, {"User-Agent","test_ua"}};
    
    auto url = "https://github.com";
    auto request = CreateRequest(url, header);
    std::shared_ptr<Response> response1 = nullptr;
    
    auto url2 = "https://www.youtube.com";
    auto request2 = CreateRequest(url2, header);
    std::shared_ptr<Response> response2 = nullptr;
    
    auto url3 = "http://bing.com";
    auto request3 = CreateRequest(url3, header);
    std::shared_ptr<Response> response3 = nullptr;
    
    std::string url4 = "http://www.cplusplus.com/doc/tutorial/";
    auto request4 = CreateRequest(url4, header);
    std::shared_ptr<Response> response4 = nullptr;
    
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<int> request_count(-4);
    
    client->PerformRequest(request, [&](const std::shared_ptr<Response> &response) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            request_count++;
            response1 = response;
        }
        cv.notify_one();
    });
    
    client->PerformRequest(request2, [&](const std::shared_ptr<Response> &response) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            request_count ++;
            response2 = response;
        }
        cv.notify_one();
    });
    
    client->PerformRequest(request3, [&](const std::shared_ptr<Response> &response) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            request_count ++;
            response3 = response;
        }
        cv.notify_one();
    });
    
    client->PerformRequest(request4, [&](const std::shared_ptr<Response> &response) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            request_count ++;
            response4 = response;
        }
        cv.notify_one();
    });
    
    
    std::unique_lock<std::mutex> lock(mutex);
    while (request_count < 0) {
        cv.wait(lock);
    }
    
    EXPECT_NE(response1.get(), nullptr);
    EXPECT_NE(response2.get(), nullptr);
    EXPECT_NE(response3.get(), nullptr);
    
    LOGD("request url:%s response code:%d", response1->request()->url().c_str(), response1->statusCode());
    LOGD("request url:%s response code:%d", response2->request()->url().c_str(), response2->statusCode());
    LOGD("request url:%s response code:%d", response3->request()->url().c_str(), response3->statusCode());
    LOGD("request url:%s response code:%d", response4->request()->url().c_str(), response4->statusCode());
}

