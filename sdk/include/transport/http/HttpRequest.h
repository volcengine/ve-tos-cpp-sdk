#pragma once

#include "common/Common.h"
#include "Url.h"
#include <memory>
#include <sstream>
#include <string>

namespace VolcengineTos {

class HttpRequest {
public:
  HttpRequest(const std::string& method = "GET",
              const std::shared_ptr<std::iostream> &responseBody = std::make_shared<std::stringstream>());
  ~HttpRequest();

  void setHeader(const std::string &key, const std::string &value) {
    headers_[key] = value;
  }
  void setHeaders(const std::map<std::string, std::string> &headers) { headers_ = headers; }
  void removeHeader(const std::string &key) { headers_.erase(key); }
  bool hasHeader(const std::string &key) const {
    return headers_.find(key) != headers_.end();
  }
  std::string getHeaderValueByKey(const std::string &key) const {
    auto it = headers_.find(key);
    if (it != headers_.end())
      return it->second;
    else
      return {};
  }
  const std::map<std::string, std::string> &Headers() const {
    return headers_;
  }

  std::shared_ptr<std::iostream> &Body() { return body_; }
  void setBody(const std::shared_ptr<std::iostream> &body) {
    body_ = body;
  }

  const std::string &method() { return method_; }
  void setMethod(const std::string &method) { method_ = method; }

  Url url() const { return url_; }
  void setUrl(const Url &url) { url_ = url; }

  void setTransferedBytes(int64_t value) { transferedBytes_ = value; }
  uint64_t TransferedBytes() const { return transferedBytes_; }

  std::shared_ptr<std::iostream> responseOutput() { return responseOutput_; };

  int64_t getContentLength() const { return contentLength_; }
  void setContentLength(int64_t contentLength) {
    contentLength_ = contentLength;
  }

private:
  std::string method_;
  Url url_;
  int64_t transferedBytes_;
  int64_t contentLength_ = 0;
  std::map<std::string, std::string> headers_;
  std::shared_ptr<std::iostream> body_;
  std::shared_ptr<std::iostream> responseOutput_;
};
} // namespace VolcengineTos
