//
// Created by zerdzhong on 2020/1/31.
//

#ifndef FFNETWORK_URL_H
#define FFNETWORK_URL_H

#include <string>

namespace ffnetwork {

class Url {
public:
  Url(std::string  url);
  ~Url();

  std::string scheme() const;
  std::string host() const;
  std::string port() const;
  std::string path() const;
  std::string query() const;

  std::string absoluteString() const ;

private:
  bool ParseUrl();

private:
  std::string scheme_;
  std::string host_;
  std::string port_;
  std::string path_;
  std::string query_;

  std::string url_;
  void* url_handle_ = nullptr;
};

}//namespace ffnetwork

#endif // FFNETWORK_URL_H
