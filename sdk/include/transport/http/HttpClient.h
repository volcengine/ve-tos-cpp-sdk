#pragma once

#include <memory>

#include "HttpRequest.h"
#include "HttpResponse.h"

namespace VolcengineTos {
static bool hasInitHttpClient = false;
struct HttpConfig {
  int requestTimeout;
  int dialTimeout;
  int tcpKeepAlive;
  int connectTimeout;
};
class HttpClient {
public:
  HttpClient(){
    if(!hasInitHttpClient){
      initGlobalState();
      hasInitHttpClient = true;
    }
  }
  HttpClient(const HttpConfig &config);
  virtual ~HttpClient(){
    cleanupGlobalState();
    hasInitHttpClient = false;
  }

  static void initGlobalState();
  static void cleanupGlobalState();

  std::shared_ptr<HttpResponse>
  doRequest(const std::shared_ptr<HttpRequest> &request);

private:
  int requestTimeout_;
  int dialTimeout_;
  int tcpKeepAlive_;
  int connectTimeout_;
};
} // namespace VolcengineTos