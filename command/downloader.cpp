//
// Created by zerdzhong on 2020/1/29.
//

#include <string>
#include <ffnetwork/client.h>
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>

#include "file.h"
#include "mapping.h"

namespace fftool {

class Downloader : public std::enable_shared_from_this<Downloader>,
                   public ffnetwork::RequestTaskDelegate {
public:
  Downloader(const std::string& url, const std::string& path) :
  url_(url),
  path_(path),
  download_finish_(false),
  output_fd_(ffbase::OpenFile(path_.c_str(), true, ffbase::FilePermission::kReadWrite))
  {
    FF_DCHECK(output_fd_.is_valid());
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
                    std::shared_ptr<ffnetwork::Response> response) override {
    ffbase::TruncateFile(output_fd_, response->expectedContentLength());
    if (!output_file_mapping_) {
      output_file_mapping_ = std::unique_ptr<ffbase::FileMapping>(new ffbase::FileMapping(
          output_fd_, {ffbase::FileMapping::Protection::kWrite}));
    }
  }
  void OnReceiveData(ffnetwork::RequestTask *task, char *data,
                     size_t length) override {
    auto * mapping = output_file_mapping_->GetMutableMapping();
    ::memcpy(mapping + offset_, data, length);
    offset_ += length;
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
  int offset_ = 0;

  ffbase::UniqueFD output_fd_;
  std::unique_ptr<ffbase::FileMapping> output_file_mapping_;
};

}//namespace fftool