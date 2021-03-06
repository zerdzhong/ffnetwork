//
// Created by zerdzhong on 2019/9/4.
//

#include "base/logging.h"
#include "ffnetwork/client.h"
#include "gtest/gtest.h"
#include <atomic>
#include <condition_variable>
#include <mutex>

using namespace ffnetwork;

TEST(CurlClientTests, create) {
  auto client = CreateClient();
  EXPECT_NE((void *)0, client.get());
}

TEST(CurlClientTests, request) {
  auto client = CreateClient();
  EXPECT_NE((void *)0, client.get());

  auto url = "https://bing.com";
  std::unordered_map<std::string, std::string> header = {
      {"Range", "0-"}, {"User-Agent", "test_ua"}};
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

  std::unordered_map<std::string, std::string> header = {
      {"Range", "0-"}, {"User-Agent", "test_ua"}};

  auto url = "https://www.apple.com";
  auto request = CreateRequest(url, header);
  std::shared_ptr<Response> response1 = nullptr;

  auto url2 = "http://llvm.org/";
  auto request2 = CreateRequest(url2, header);
  std::shared_ptr<Response> response2 = nullptr;

  auto url3 = "http://bing.com";
  auto request3 = CreateRequest(url3, header);
  std::shared_ptr<Response> response3 = nullptr;

  std::mutex mutex;
  std::condition_variable cv;
  std::atomic<int> request_count(-3);

  client->PerformRequest(request,
                         [&](const std::shared_ptr<Response> &response) {
                           {
                             std::lock_guard<std::mutex> lock(mutex);
                             request_count++;
                             response1 = response;
                           }
                           cv.notify_one();
                         });

  client->PerformRequest(request2,
                         [&](const std::shared_ptr<Response> &response) {
                           {
                             std::lock_guard<std::mutex> lock(mutex);
                             request_count++;
                             response2 = response;
                           }
                           cv.notify_one();
                         });

  client->PerformRequest(request3,
                         [&](const std::shared_ptr<Response> &response) {
                           {
                             std::lock_guard<std::mutex> lock(mutex);
                             request_count++;
                             response3 = response;
                           }
                           cv.notify_one();
                         });

  std::unique_lock<std::mutex> lock(mutex);
  while (request_count < 0) {
    cv.wait(lock);
  }

  FF_LOG_P(DEBUG, "%s response1 metrics :%s",
           response1->request()->url().c_str(),
           metrics_dump_info(response1->metrics().get()).c_str());
  FF_LOG_P(DEBUG, "%s response2 metrics :%s",
           response2->request()->url().c_str(),
           metrics_dump_info(response2->metrics().get()).c_str());
  FF_LOG_P(DEBUG, "%s response3 metrics :%s",
           response3->request()->url().c_str(),
           metrics_dump_info(response3->metrics().get()).c_str());

  EXPECT_NE(response1.get(), nullptr);
  EXPECT_NE(response2.get(), nullptr);
  EXPECT_NE(response3.get(), nullptr);

  EXPECT_NE(response1->responseCode(), ResponseCode::Invalid);
  EXPECT_NE(response2->responseCode(), ResponseCode::Invalid);
  EXPECT_NE(response3->responseCode(), ResponseCode::Invalid);
}

TEST(CurlClientTests, Cancel) {
  auto client = CreateClient();
  EXPECT_NE((void *)0, client.get());

  auto url = "https://bing.com";
  auto request = CreateRequest(url, {});
  std::shared_ptr<Response> cancelled_response = nullptr;

  std::mutex mutex;
  std::condition_variable cv;
  std::atomic<bool> request_done(false);

  auto request_task = client->PerformRequest(
      request, [&](const std::shared_ptr<Response> &response) {
        {
          std::lock_guard<std::mutex> lock(mutex);
          request_done = true;
          cancelled_response = response;
        }
        cv.notify_one();
      });

  request_task->Cancel();

  std::unique_lock<std::mutex> lock(mutex);
  while (!request_done) {
    cv.wait(lock);
  }

  EXPECT_EQ(request_task->isCancelled(), true);
  EXPECT_NE((void *)0, cancelled_response.get());
  EXPECT_EQ(cancelled_response->cancelled(), true);
  EXPECT_EQ(cancelled_response->responseCode(), ResponseCode::UserCancel);
  EXPECT_EQ(cancelled_response->statusCode(),
            HttpStatusCode::StatusCodeInvalid);
}
