#include <ffnetwork/Request.h>

namespace ffnetwork {

    class RequestImpl : public Request {
    public:
        RequestImpl(const std::string &url,
        const std::unordered_map<std::string, std::string> &header_map);
        RequestImpl(const Request &request);
        virtual ~RequestImpl();

        std::string url() const override;
        void setUrl(const std::string &url) override;
        
        std::unordered_map<std::string, std::string> &headerMap() override;
        std::unordered_map<std::string, std::string> headerMap() const override;

        std::string method() const override;
        void setMethod(const std::string &method) override;

        const unsigned char *data(size_t &data_length) const override;
        void setData(const unsigned char *data, size_t data_length) override;

        std::string hash() const override;
        std::string serialise() const override;
    private:
        std::string url_;
        std::unordered_map<std::string, std::string> headers_;
        std::string method_;
        unsigned char* data_;
        size_t data_length_;
    };
}//end of ffnetwork