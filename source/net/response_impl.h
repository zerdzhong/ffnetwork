//
// Created by zerdzhong on 2019/9/9.
//

#ifndef FFNETWORK_RESPONSE_IMPL_H
#define FFNETWORK_RESPONSE_IMPL_H

#include <ffnetwork/request.h>
#include <ffnetwork/response.h>
#include <memory>

namespace ffnetwork {

class ResponseImpl : public Response {
public:
  ResponseImpl();
  ResponseImpl(const std::shared_ptr<Request> &request,
               HttpStatusCode status_code, ResponseCode response_code,
               const std::shared_ptr<Metrics> &metrics, bool cancelled);
  ResponseImpl(const std::string &serialised, const unsigned char *data,
               size_t data_length,
               const std::shared_ptr<Response> &response = nullptr);

  virtual ~ResponseImpl();

  void Construct(const std::shared_ptr<Request> &request,
                 HttpStatusCode status_code, ResponseCode response_code,
                 const std::shared_ptr<Metrics> &metrics);

  // Response
  std::shared_ptr<Request> request() const override;
  std::shared_ptr<Metrics> metrics() const override;

  std::string url() const override ;
  void setUrl(const std::string& url);

  HttpStatusCode statusCode() const override;
  ResponseCode responseCode() const override;

  uint64_t expectedContentLength() const override ;
  std::string mimeType() const override ;
  std::string encodingType() const override ;
  std::string suggestedFileName() const override ;

  bool cancelled() const override;
  std::string serialise() const override;
  std::unordered_map<std::string, std::string> metadata() const override;
  void setMetadata(const std::string &key, const std::string &value) override;

  std::string operator[](const std::string &header_name) const override;
  std::string &operator[](const std::string &header_name) override;
  std::unordered_map<std::string, std::string> &headerMap() override;
  std::unordered_map<std::string, std::string> headerMap() const override;

  void UpdateHeader(const std::string& key, const std::string& value);

private:
  std::shared_ptr<Request> request_;
  std::shared_ptr<Metrics> metrics_;
  HttpStatusCode status_code_;
  ResponseCode response_code_;
  const bool cancelled_;
  std::unordered_map<std::string, std::string> headers_;
  std::unordered_map<std::string, std::string> metadata_;

  uint64_t expected_content_length_ = 0;
  std::string encoding_type_;
  std::string mime_type_;
  std::string suggested_file_name_;

  std::string url_;
};

} // namespace ffnetwork

#endif // FFNETWORK_RESPONSE_IMPL_H
