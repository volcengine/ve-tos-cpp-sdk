#pragma once

#include <string>
#include <map>
namespace VolcengineTos {

class RequestInfo {
public:
  const std::string &getRequestId() const { return requestId_; }
  void setRequestId(const std::string &requestId) { requestId_ = requestId; }
  const std::map<std::string, std::string> &getHeaders() const { return headers_; }
  void setHeaders(const std::map<std::string, std::string> &headers) { headers_ = headers; }

private:
  std::string requestId_;
  std::map<std::string, std::string> headers_;
};
} // namespace vocl_tos