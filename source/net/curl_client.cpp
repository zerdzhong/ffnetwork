//
// Created by zerdzhong on 2019-06-23.
//

#include "curl_client.h"
#include "base/logging.h"
#include "base/messageloop/message_loop.h"
#include "base/thread/mutex.h"
#include "request_task_impl.h"
#include "utils/time_utils.h"
#include <cstring>
#include <net/response_impl.h>
#include <unistd.h>
#include <vector>

namespace {
void ConfigCurlGlobalState(bool added_client) {
  auto *mutex = ffbase::SharedMutex::Create();
  ffbase::UniqueLock lock(*mutex);

  static long curl_clients_active = 0;

  long previous_curl_clients_active = curl_clients_active;
  if (added_client) {
    curl_clients_active++;
  } else if (curl_clients_active != 0) {
    curl_clients_active--;
  }
  if (previous_curl_clients_active == 0 && curl_clients_active == 1) {
    curl_global_init(CURL_GLOBAL_ALL);
  } else if (previous_curl_clients_active == 1 && curl_clients_active == 0) {
    curl_global_cleanup();
  }
}
} // namespace

namespace ffnetwork {

std::shared_ptr<Client> CreateCurlClient() {
  auto curl_client = std::make_shared<CurlClient>();
  return curl_client;
}

#pragma mark - CurlClient
CurlClient::CurlClient() :
is_terminated_(false),
use_multi_wait_(true),
request_thread_("CurlClient")
{
  ConfigCurlGlobalState(true);
  curl_multi_handle_ = curl_multi_init();
  curl_multi_setopt(curl_multi_handle_, CURLMOPT_MAXCONNECTS, MAX_CONNECTIONS);
  request_thread_.Start([this] { this->Run(); });
}

CurlClient::~CurlClient() {

  is_terminated_ = true;
  req_condition_.notify_all();
  request_thread_.Join();

  std::unique_lock<std::mutex> client_lock(client_mutex_);

  // Remove any remaining requests
  std::vector<std::string> hashes;
  for (auto &p : request_task_map_) {
    hashes.push_back(p.first);
    curl_multi_remove_handle(curl_multi_handle_, p.second->handle());
  }
  for (const auto &h : hashes) {
    CleanupRequest(h);
  }

  curl_multi_cleanup(curl_multi_handle_);
  ConfigCurlGlobalState(false);
  client_lock.unlock();
}

#pragma mark - public_method

std::shared_ptr<RequestTask>
CurlClient::PerformRequest(const std::shared_ptr<Request> &request,
                           CompletionCallback callback) {

  FF_LOG_P(INFO, "PerformRequest url %s", request->url().c_str());

  auto request_task = TaskWithRequest(request, callback);
  request_task->Resume();

  return request_task;
}

std::shared_ptr<RequestTask>
CurlClient::TaskWithRequest(const std::shared_ptr<Request> &request,
                            CompletionCallback callback) {

  std::unique_lock<std::mutex> client_lock(client_mutex_);
  std::string request_hash = request->hash();

  auto request_task =
      std::make_shared<RequestTaskImpl>(request, shared_from_this());
  request_task->setCompletionCallback(callback);

  request_task_map_[request_hash] = request_task;

  return request_task;
}

#pragma mark - RequestTaskDelegate

void CurlClient::RequestTaskCancel(const std::shared_ptr<RequestTask> &task) {
  std::string request_hash = task->taskIdentifier();
  request_thread_.GetTaskRunner()->PostTask(
      std::bind(&CurlClient::CancelRequest, this, request_hash));
}

void CurlClient::RequestTaskStart(const std::shared_ptr<RequestTask> &task) {
  std::string request_hash = task->taskIdentifier();
  request_thread_.GetTaskRunner()->PostTask(
      std::bind(&CurlClient::StartRequest, this, request_hash));
}

#pragma mark - run_entry

void CurlClient::Run() {
  int active_requests = -1;

  ffbase::MessageLoop::EnsureInitializedForCurrentThread();
  auto &loop = ffbase::MessageLoop::GetCurrent();

  while (!is_terminated_) {
    curl_multi_perform(curl_multi_handle_, &active_requests);

    if (HandleCurlMsg()) {
      continue;
    }

    loop.RunForTime(ffbase::TimeDelta::FromMilliseconds(10));

    if (active_requests > 0) {
      if (use_multi_wait_) {
        WaitMulti(100);
      } else {
        WaitFD(100);
      }
    } else if (is_terminated_) {
      return;
    }
  }
}

#pragma mark - private_method

void CurlClient::CleanupRequest(const std::string &hash) {
  request_task_map_.erase(hash);
}

bool CurlClient::HandleCurlMsg() {

  CURLMsg *msg;
  int message_in_queue = -1;

  size_t msg_count = 0;

  while ((msg = curl_multi_info_read(curl_multi_handle_, &message_in_queue))) {
    ++msg_count;

    if (msg->msg == CURLMSG_DONE) {

      std::string *request_hash;
      CURL *handle = msg->easy_handle;

      curl_easy_getinfo(handle, CURLINFO_PRIVATE, &request_hash);

      if (msg->data.result != CURLE_OK) {
        // TODO retry?
        FF_LOG(ERROR) << "CURL Error " << curl_easy_strerror(msg->data.result);
      }

      ResponseCode response_code = ResponseCode::Invalid;
      response_code = ConvertCurlCode(msg->data.result);

      // make response to send to callback
      long status_code = 0;
      curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status_code);

      // Look up response data and original request
      auto task = request_task_map_[*request_hash];
      task->DidFinished(HttpStatusCode(status_code), response_code);

      curl_multi_remove_handle(curl_multi_handle_, handle);
      CleanupRequest(*request_hash);

    } else {
      FF_LOG(ERROR) << "CURLMsg " << msg->msg;
    }
  }

  return msg_count > 0;
}

ResponseCode CurlClient::ConvertCurlCode(CURLcode code) {
  ResponseCode response_code;
  switch (code) {
  case CURLE_OK: {
    response_code = ResponseCode::OK;
    break;
  }
  case CURLE_COULDNT_CONNECT: {
    response_code = ResponseCode::ConnectToServerFailed;
    break;
  }
  case CURLE_OPERATION_TIMEDOUT: {
    response_code = ResponseCode::Timeout;
    break;
  }
  default:
    response_code = ResponseCode::UnknownError;
    break;
  }

  return response_code;
}

void CurlClient::WaitMulti(long timeout_ms) {
  int num_fds = -1;
  CURLMcode res =
      curl_multi_wait(curl_multi_handle_, nullptr, 0, timeout_ms, &num_fds);
  if (res != CURLM_OK) {
    FF_LOG_P(ERROR, "curl_multi_wait not ok err_code:%d, desc:%s\n", res,
             curl_multi_strerror(res));
  }
}

void CurlClient::WaitFD(long timeout_ms) {
  int max_fd = -1;
  long wait_time = -1;
  struct timeval T {};

  fd_set R, W, E;

  FD_ZERO(&R);
  FD_ZERO(&W);
  FD_ZERO(&E);

  if (curl_multi_fdset(curl_multi_handle_, &R, &W, &E, &max_fd)) {
    FF_LOG_P(ERROR, "E: curl_multi_fdset\n");
  }

  if (curl_multi_timeout(curl_multi_handle_, &wait_time)) {
    FF_LOG_P(ERROR, "E: curl_multi_timeout\n");
  }

  if (wait_time == -1) {
    wait_time = timeout_ms;
  }

  if (max_fd == -1) {
    usleep((unsigned long)wait_time);
  } else {
    T.tv_sec = wait_time / 1000;
    T.tv_usec = (wait_time % 1000) * 1000;

    if (0 > select(max_fd + 1, &R, &W, &E, &T)) {
      FF_LOG_P(ERROR, "E: select(%i,,,,%li): %i: %s\n", max_fd + 1, wait_time,
               errno, strerror(errno));
    }
  }
}

void CurlClient::CancelRequest(const std::string &hash) {

  if (request_task_map_.find(hash) == request_task_map_.end()) {
    return;
  }

  auto &request_task = request_task_map_[hash];
  request_task->DidCancelled();

  curl_multi_remove_handle(curl_multi_handle_, request_task->handle());
  CleanupRequest(hash);
}

void CurlClient::StartRequest(const std::string &hash) {

  if (request_task_map_.find(hash) == request_task_map_.end()) {
    return;
  }

  auto &request_task = request_task_map_[hash];

  curl_multi_add_handle(curl_multi_handle_, request_task->handle());
}

} // namespace ffnetwork
