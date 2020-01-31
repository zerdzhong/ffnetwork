//
// Created by zerdzhong on 2020/1/31.
//

#include "url.h"
#include <utility>
#include <curl/urlapi.h>

namespace ffnetwork {

Url::Url(std::string url) : url_(std::move(url)) {
  url_handle_ = curl_url();
  ParseUrl();
}

std::string Url::scheme() const { return scheme_; }
std::string Url::host() const { return host_; }
std::string Url::port() const { return port_; }
std::string Url::path() const { return path_; }
std::string Url::query() const { return query_; }
std::string Url::absoluteString() const { return url_; }

bool Url::ParseUrl() {
  auto *url_handle = static_cast<CURLU *>(url_handle_);
  CURLUcode uc;

  if (!url_handle) {
    return false;
  }

  uc = curl_url_set(url_handle, CURLUPART_URL, url_.c_str(), 0);
  if (uc) {
    return false;
  }

  char* result;

  uc = curl_url_get(url_handle, CURLUPART_SCHEME, &result, 0);
  if (!uc) {
    scheme_ = std::string (result);
    curl_free(result);
  }

  uc = curl_url_get(url_handle, CURLUPART_HOST, &result, 0);
  if (!uc) {
    host_ = std::string (result);
    curl_free(result);
  }

  uc = curl_url_get(url_handle, CURLUPART_PORT, &result, 0);
  if (uc == CURLUE_NO_PORT) {
    port_ = "80";
  } else {
    port_ = std::string (result);
    curl_free(result);
  }

  uc = curl_url_get(url_handle, CURLUPART_PATH, &result, 0);
  if (!uc) {
    path_ = std::string (result);
    curl_free(result);
  }

  uc = curl_url_get(url_handle, CURLUPART_QUERY, &result, 0);
  if (!uc) {
    query_ = std::string (result);
    curl_free(result);
  }

  return true;
}

Url::~Url() {
  if (url_handle_) {
    curl_url_cleanup(static_cast<CURLU *>(url_handle_));
  }
}

}//namespace ffnetwork

