//
// Created by zerdzhong on 2020/1/29.
//

#include <string>
#include <ffnetwork/client.h>
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>

namespace fftool {

class Downloader : public std::enable_shared_from_this<Downloader>,
                   public ffnetwork::RequestTaskDelegate {
public:
  Downloader(const std::string& url, const std::string& path) :
  url_(url),
  path_(path),
  download_finish_(false)
  {
  }

  void SyncStart() {
    auto client = ffnetwork::CreateClient();
    auto request = ffnetwork::CreateRequest(url_, {});
    auto task = client->TaskWithRequest(request, nullptr);

    task->setTaskDelegate(shared_from_this());

    task->Resume();

    std::unique_lock<std::mutex> lock(mutex_);
    while (!download_finish_) {
      cv_.wait(lock);
    }

  }

  void AsyncStart(std::function<void()> complete_callback) {

  }
#pragma mark- RequestTaskDelegate
  void
  OnReceiveResponse(ffnetwork::RequestTask *task,
                    std::shared_ptr<ffnetwork::Response> response) override {}
  void OnReceiveData(ffnetwork::RequestTask *task, char *data,
                     size_t length) override {
    std::cout << "OnReceiveData :" << data;
  }
  void OnRequestTaskComplete(ffnetwork::RequestTask *task,
                             ffnetwork::ResponseCode result) override {
    std::unique_lock<std::mutex> lock(mutex_);
    download_finish_ = true;
    cv_.notify_one();
  }

private:
  std::string url_;
  std::string path_;

  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic<bool> download_finish_;
};

}//namespace fftool
