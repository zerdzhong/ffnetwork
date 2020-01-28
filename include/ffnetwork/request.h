#ifndef FFNETWORK_REQUEST_H
#define FFNETWORK_REQUEST_H

#include <memory>
#include <string>
#include <unordered_map>

namespace ffnetwork {
extern const std::string GetMethod;
extern const std::string PostMethod;
extern const std::string PutMethod;
extern const std::string HeadMethod;

class Request {
public:
  virtual std::string url() const = 0;
  virtual void setUrl(const std::string &url) = 0;

  virtual std::unordered_map<std::string, std::string> &headerMap() = 0;
  virtual std::unordered_map<std::string, std::string> headerMap() const = 0;

  virtual std::string method() const = 0;
  virtual void setMethod(const std::string &method) = 0;

  virtual const unsigned char *data(size_t &data_length) const = 0;
  virtual void setData(const unsigned char *data, size_t data_length) = 0;

  virtual std::string hash() const = 0;
  virtual std::string serialise() const = 0;
};

extern std::shared_ptr<Request>
CreateRequest(const std::string &url,
              const std::unordered_map<std::string, std::string> &header_map);
extern std::shared_ptr<Request>
CreateRequest(const std::shared_ptr<Request> &request);

} // end of namespace ffnetwork

#endif