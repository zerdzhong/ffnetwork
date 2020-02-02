//
// Created by zerdzhong on 2020/1/29.
//

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <ffnetwork/client.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include "file.h"
#include "mapping.h"
#include "net/url.h"

namespace fftool {

class Downloader : public std::enable_shared_from_this<Downloader>,
                   public ffnetwork::RequestTaskDelegate {
public:
  Downloader(std::string url, std::string path)
      : url_(std::move(url)), path_(std::move(path)), download_finish_(false) {
    ConstructPath();
    output_file_mapping_ =
        std::make_unique<ffbase::AutoSyncFileMapping>(path_.c_str());
    FF_DCHECK(output_file_mapping_->IsValid());
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

  void ConstructPath() {

    if (path_.empty()) {
      path_ = "./"; // use current path
    }

    if (path_[path_.size() - 1] != '/') {
      return;
    }

    auto url_path = ffnetwork::Url(url_).path();
    auto last_slash_pos = url_path.find_last_of('/');

    std::string file_name;

    if (last_slash_pos != std::string::npos) {
      file_name = url_path.substr(last_slash_pos + 1);
    } else {
      file_name = url_path;
    }

    path_ += file_name;
  }
#pragma mark - RequestTaskDelegate
  void
  OnReceiveResponse(ffnetwork::RequestTask *task,
                    std::shared_ptr<ffnetwork::Response> response) override {
    for (auto& header : response->headerMap()) {
      FF_LOG(DEBUG) << header.first << ": " << header.second;
    }
  }

  void OnReceiveData(ffnetwork::RequestTask *task, char *data,
                     size_t length) override {
    output_file_mapping_->AppendData((uint8_t *)data, length);
    offset_ += length;

//    FF_LOG(DEBUG) << "OnReceiveData: " << offset_;
  }

  void OnRequestTaskComplete(ffnetwork::RequestTask *task,
                             std::shared_ptr<ffnetwork::Response> response,
                             ffnetwork::ErrorCode result) override {
    std::unique_lock<std::mutex> lock(mutex_);
    download_finish_ = true;
    cv_.notify_one();

    FF_LOG(DEBUG) << ffnetwork::metrics_dump_info(response->metrics().get());

  }

private:
  std::string url_;
  std::string path_;

  std::mutex mutex_;
  std::condition_variable cv_;
  std::atomic<bool> download_finish_;
  int offset_ = 0;

  std::unique_ptr<ffbase::AutoSyncFileMapping> output_file_mapping_;
};

} // namespace fftool
