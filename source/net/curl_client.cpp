//
// Created by zerdzhong on 2019-06-23.
//

#include "curl_client.h"
#include <sstream>
#include <vector>
#include <utility>
#include <unistd.h>
#include "log/log_macro.h"
#include "request_task_impl.h"
#include <ffnetwork/response_impl.h>

namespace {
    void ConfigCurlGlobalState(bool added_client) {
        ffnetwork::CriticalSection criticalSection;
        ffnetwork::CriticalScope cs(&criticalSection);

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
}

namespace ffnetwork {

    std::shared_ptr<Client> CreateCurlClient() {
        auto curl_client = std::make_shared<CurlClient>();
        return curl_client;
    }

#pragma mark- CurlClient
    CurlClient::CurlClient() : have_new_request_(false), is_terminated_(false), use_multi_wait_(true){
        ConfigCurlGlobalState(true);
        curl_multi_handle_ = curl_multi_init();
        curl_multi_setopt(curl_multi_handle_, CURLMOPT_MAXCONNECTS, MAX_CONNECTIONS);
        request_thread_.SetName("CurlClient", this);
        request_thread_.Start(this);
    }

    CurlClient::~CurlClient() {
        std::unique_lock<std::mutex> client_lock(client_mutex_);

        // Remove any remaining requests
        std::vector<std::string> hashes;
        for (auto &p : handles_) {
            hashes.push_back(p.first);
            curl_multi_remove_handle(curl_multi_handle_, p.second->handle);
        }
        for (const auto& h : hashes) {
            CleanupRequest(h);
        }

        curl_multi_cleanup(curl_multi_handle_);
        ConfigCurlGlobalState(false);
        is_terminated_ = true;
        client_lock.unlock();

        new_req_condition_.notify_all();
        request_thread_.Stop();
    }

#pragma mark- public_method

    std::shared_ptr<RequestTask> CurlClient::PerformRequest(const std::shared_ptr<Request> &request, std::function<void(
            const std::shared_ptr<Response> &)> callback) {
        
        LOGD("PerformRequest url:%s", request->url().c_str());

        std::unique_lock<std::mutex> client_lock(client_mutex_);
        std::string request_hash = request->hash();

        std::shared_ptr<RequestTask> request_task =
                std::make_shared<RequestTaskImpl>(shared_from_this(), request_hash);

        handles_[request_hash] = std::unique_ptr<HandleInfo>(new HandleInfo(request, callback));
        std::unique_ptr<HandleInfo> &handle_info = handles_[request_hash];

        curl_multi_add_handle(curl_multi_handle_, handle_info->handle);

        have_new_request_ = true;
        client_lock.unlock();
        new_req_condition_.notify_one();

        return request_task;
    }

    void CurlClient::RequestTaskDidCancel(const std::shared_ptr<RequestTask> &task) const {
        
    }


#pragma mark- run_entry

    void CurlClient::Run(Thread *thread) {

        std::unique_lock<std::mutex> client_lock(client_mutex_);

        int active_requests = -1;

        while (!thread->IsQuitting()) {
            curl_multi_perform(curl_multi_handle_, &active_requests);

            if (HandleCurlMsg()) {
                continue;
            }

            if (active_requests > 0) {
                if (use_multi_wait_) {
                    WaitMulti(100);
                } else {
                    WaitFD(100);
                }
            } else {
                have_new_request_ = false;
                new_req_condition_.wait(client_lock, [this]() {
                    return (have_new_request_ || is_terminated_);
                });

                if (is_terminated_) {
                    return;
                }
            }
        }
    }

#pragma mark- private_method

    void CurlClient::CleanupRequest(const std::string& hash) {
        handles_.erase(hash);
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

                if(msg->data.result != CURLE_OK){
                    // TODO retry?
                    LOGE("CURL Error (%s)", curl_easy_strerror(msg->data.result));
                }

                ResponseCode response_code = ResponseCode::Invalid;
                response_code = ConvertCurlCode(msg->data.result);
                
                // make response to send to callback
                long status_code = 0;
                curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &status_code);

                // Look up response data and original request
                HandleInfo *handle_info = handles_[*request_hash].get();
                const std::shared_ptr<Request> request = handle_info->request;
                const auto *data = (const unsigned char *) handle_info->response.c_str();
                size_t data_length = handle_info->response.size();

                LOGD("Got response for: %s", request->url().c_str());
                LOGD("Response code: %lu", status_code);
                LOGD("Response size: %lu", data_length);

                std::shared_ptr<Response> new_response = std::make_shared<ResponseImpl>(
                        request, data, data_length, HttpStatusCode(status_code), response_code, false);

                auto &response_headers = new_response->headerMap();
                response_headers = std::move(handle_info->response_headers);

                // Save callback before cleanup
                auto cb = handle_info->callback;
                curl_multi_remove_handle(curl_multi_handle_, handle);
                CleanupRequest(*request_hash);

                if (cb) {
                    cb(new_response);
                }
            } else {
                LOGE("CURLMsg (%d)\n", msg->msg);
            }
        }

        return msg_count > 0;
    }
    
    ResponseCode CurlClient::ConvertCurlCode(CURLcode code) {
        ResponseCode response_code = ResponseCode::Invalid;
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
        int res = curl_multi_wait(curl_multi_handle_, NULL, 0, timeout_ms, &num_fds);
        if(res != CURLM_OK) {
            LOGE("curl_multi_wait not ok (%d)\n", res);
        }
    }

    void CurlClient::WaitFD(long timeout_ms) {
        int max_fd = -1;
        long wait_time = -1;
        struct timeval T;

        fd_set R, W, E;

        FD_ZERO(&R);
        FD_ZERO(&W);
        FD_ZERO(&E);

        if (curl_multi_fdset(curl_multi_handle_, &R, &W, &E, &max_fd)) {
            LOGE("E: curl_multi_fdset\n");
        }

        if (curl_multi_timeout(curl_multi_handle_, &wait_time)) {
            LOGE("E: curl_multi_timeout\n");
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
                LOGE("E: select(%i,,,,%li): %i: %s\n", max_fd + 1, wait_time, errno, strerror(errno));
            }
        }
    }


#pragma mark- CurlCallback

    size_t CurlClient::write_callback(char *data, size_t size, size_t nitems, void *str) {
        auto* string_buffer = static_cast<std::string *>(str);
        if(string_buffer == nullptr) {
            return 0;
        }
        string_buffer->append(data, size * nitems);
        return size * nitems;
    }

    size_t CurlClient::header_callback(char *data, size_t size, size_t nitems, void *str) {
        auto headers = reinterpret_cast<std::unordered_map<std::string, std::string> *>(str);
        std::string header(data, size * nitems), key, value;
        size_t pos;
        if ((pos = header.find(':')) != std::string::npos) {
            key = header.substr(0, pos);
            value = header.substr(std::min(pos + 2, header.length()));
        }

        if (!key.empty()) {
            (*headers)[key] = value;
        }

        return size*nitems;
    }

#pragma mark- HandleInfo

    CurlClient::HandleInfo::HandleInfo() :
    handle(nullptr),
    request(nullptr),
    request_headers(nullptr),
    callback(nullptr)
    {

    }

    CurlClient::HandleInfo::HandleInfo(std::shared_ptr<Request> req,
                                       std::function<void(const std::shared_ptr<Response> &)> callback)
                                       :request(req), request_headers(nullptr), callback(std::move(callback))
    {
        handle = curl_easy_init();
        request_hash = request->hash();
        ConfigureCurlHandle();
    }

    CurlClient::HandleInfo::~HandleInfo() {

        if (request_headers) {
            curl_slist_free_all(request_headers);
        }

        if (handle) {
            curl_easy_cleanup(handle);
        }
    }

    void CurlClient::HandleInfo::ConfigureCurlHandle() {

        curl_easy_setopt(handle, CURLOPT_URL, request->url().c_str());
        curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(handle, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_callback);
        curl_easy_setopt(handle, CURLOPT_WRITEHEADER, &response_headers);

        curl_easy_setopt(handle, CURLOPT_TIMEOUT, 30);

#if __APPLE__
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, false);
#endif

        // Stash request hash in a pointer so we can get callback later
        curl_easy_setopt(handle, CURLOPT_PRIVATE, &request_hash);

        // Set method
        if (request->method() == GetMethod) {
            curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
        } else if (request->method() == PutMethod) {
            curl_easy_setopt(handle, CURLOPT_PUT, 1);
        } else if (request->method() == PostMethod) {
            curl_easy_setopt(handle, CURLOPT_POST, 1);
        } else if (request->method() == HeadMethod) {
            curl_easy_setopt(handle, CURLOPT_HTTPGET, 1);
            curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
        } else {
            curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, request->method().c_str());
        }

        // Set data
        size_t data_length;
        const unsigned char *request_data = request->data(data_length);
        if (data_length) {
            curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, data_length);
            curl_easy_setopt(handle, CURLOPT_POSTFIELDS, request_data);
        }

        // Set custom headers
        ConfigureHeaders();
        curl_easy_setopt(handle, CURLOPT_HEADEROPT, CURLHEADER_UNIFIED);
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, request_headers);
    }

    void CurlClient::HandleInfo::ConfigureHeaders() {
        struct curl_slist *headers = nullptr;
        for (auto const &header : request->headerMap()) {
            if (header.first == "Range") {
                curl_easy_setopt(handle, CURLOPT_RANGE, header.second.c_str());
                continue;
            }

            std::stringstream header_ss;
            header_ss << header.first << ": " << header.second;
            headers = curl_slist_append(headers, header_ss.str().c_str());
        }

        request_headers = headers;
    }
}
