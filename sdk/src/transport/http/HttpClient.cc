#include <iostream>
#include "curl/curl.h"

#include "transport/http/HttpClient.h"
#include "common/Common.h"
#include "utils/BaseUtils.h"
#include "TosClient.h"

namespace VolcengineTos {
struct ResourceManager {
  HttpClient *client;
  CURL *curl;
  HttpRequest *httpReq;
  HttpResponse *httpResp;
  int64_t send;   // request body send
  int64_t total;  // request body content-length
  bool firstRecv;
};

static size_t sendBody(char *ptr, size_t size, size_t nmemb, void *data) {
  auto *resourceMan = static_cast<ResourceManager *>(data);

  if (resourceMan == nullptr || resourceMan->httpReq == nullptr) {
    return 0;
  }

  std::shared_ptr<std::iostream> &content = resourceMan->httpReq->Body();
  const size_t wanted = size * nmemb;
  size_t got = 0;
  if (content != nullptr && wanted > 0) {
    size_t read = wanted;
    if (resourceMan->total > 0) {
      int64_t remains = resourceMan->total - resourceMan->send;
      if (remains < static_cast<int64_t>(wanted)) {
        read = static_cast<size_t>(remains);
      }
    }
    content->read(ptr, read);
    got = static_cast<size_t>(content->gcount());
  }

  resourceMan->send += got;
  return got;
}

static size_t recvBody(char *ptr, size_t size, size_t nmemb, void *userdata) {
  auto *resourceMan = static_cast<ResourceManager *>(userdata);
  const size_t wanted = size * nmemb;
  if (resourceMan == nullptr || resourceMan->httpResp == nullptr || wanted == 0) {
    return -1;
  }
  // 第一次receive response body , 初始化state->resposne->body
  // 如果200使用传入的iostream接收数据，反之生成一个stringstream接收错误信息
  if (resourceMan->firstRecv) {
    int64_t response_code = 0;
    curl_easy_getinfo(resourceMan->curl, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code / 100 == 2) {
      resourceMan->httpResp->setBody(resourceMan->httpReq->responseOutput());
    } else {
      resourceMan->httpResp->setBody(std::make_shared<std::stringstream>());
    }
    resourceMan->firstRecv = false;
  }
  std::shared_ptr<std::iostream> &content = resourceMan->httpResp->Body();
  if (content == nullptr || content->fail()) {
    return -2;
  }
  content->write(ptr, static_cast<std::streamsize>(wanted));
  if (content->bad()) {
    return -3;
  }
  return wanted;
}

static size_t recvHeaders(char *buffer, size_t size, size_t nitems, void *userdata) {
  auto *resourceMan = static_cast<ResourceManager *>(userdata);
  const size_t length = nitems * size;

  std::string line(buffer);
  auto pos = line.find(':');
  if (pos != std::string::npos) {
    size_t posEnd = line.rfind('\r');
    if (posEnd != std::string::npos) {
      posEnd = posEnd - pos - 2;
    }
    std::string name = line.substr(0, pos);
    std::string value = line.substr(pos + 2, posEnd);
    resourceMan->httpResp->setHeader(name, value);
  }

  if (length == 2 && (buffer[0] == 0x0D) && (buffer[1] == 0x0A)) {
    if (resourceMan->httpResp->hasHeader(http::HEADER_CONTENT_LENGTH)) {
      double dval;
      curl_easy_getinfo(resourceMan->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &dval);
      resourceMan->total = (int64_t)dval;
    }
  }
  return length;
}
} // namespace VolcengineTos

using namespace VolcengineTos;

void HttpClient::initGlobalState() {
  // init twice here, check why
  curl_global_init(CURL_GLOBAL_ALL);
}

void HttpClient::cleanupGlobalState() { curl_global_cleanup(); }

HttpClient::HttpClient(const HttpConfig &config) {
  if (!hasInitHttpClient) {
    initGlobalState();
    hasInitHttpClient = true;
  }
  tcpKeepAlive_ = config.tcpKeepAlive;
  dialTimeout_ = config.dialTimeout;
  requestTimeout_ = config.requestTimeout;
  connectTimeout_ = config.connectTimeout;
}

std::shared_ptr<HttpResponse> HttpClient::doRequest(const std::shared_ptr<HttpRequest> &request) {
  // init curl for this request
  CURL *curl = curl_easy_init();

  /* enable TCP keep-alive for this request */
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
  /* keep-alive idle time to 120 seconds */
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, tcpKeepAlive_);
  /* interval time between keep-alive probes: 60 seconds */
  curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);
  curl_easy_setopt(curl, CURLOPT_NETRC, CURL_NETRC_IGNORED);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 0L);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connectTimeout_ * 1000);
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
  curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, requestTimeout_);

  // set req specific params
  auto response = std::make_shared<HttpResponse>();
  ResourceManager resourceMan = {
      this, curl, request.get(), response.get(), 0, -1, true
  };

  // set url
// std::string url = request->url().toString();
//std::cout << url << std::endl;
  curl_easy_setopt(curl, CURLOPT_URL, request->url().toString().c_str());

  // set opt for different http methods
  if (request->method() == http::MethodHead) {
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
  } else if (request->method() == http::MethodPut) {
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    // make sure httpRequest content length has been set
    curl_off_t bodySize = request->getContentLength();
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, bodySize);
  } else if (request->method() == http::MethodPost) {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  } else if (request->method() == http::MethodDelete) {
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
  }

  // add headers
  curl_slist *list = nullptr;
  auto &headers = request->Headers();
  for (const auto &p : headers) {
    if (p.second.empty())
      continue;
    std::string str(p.first);
    str.append(":").append(p.second);
    list = curl_slist_append(list, str.c_str());
  }
  // Disable Expect: 100-continue
  list = curl_slist_append(list, "Expect:");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

  // add user-agent
  curl_easy_setopt(curl, CURLOPT_USERAGENT, VolcengineTos::DefaultUserAgent().c_str());

  // set call back func
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, &resourceMan);
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, recvHeaders);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resourceMan);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, recvBody);

  if (request->method() != http::MethodPost) {
    curl_easy_setopt(curl, CURLOPT_READDATA, &resourceMan);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, sendBody);
  } else {
    // special for post body
    if (request->Body()) {
      std::stringstream ss;
      ss << request->Body()->rdbuf();
      const std::string str(ss.str());
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, str.c_str());
    }
  }

  CURLcode res = curl_easy_perform(curl);
  if (res == CURLE_COULDNT_CONNECT) {
    response->setStatus(http::Refused);
    response->setStatusMsg("connection refused");
  } else if (res != CURLE_OK) {
    response->setStatus(http::otherErr);
    response->setStatusMsg("curl error code is " + std::to_string(res));
  } else {
    response->setStatus(http::Success);
  }
  int response_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  if (response->status() == http::Refused) {
    response_code = -1;
  }
  if (response->status() == http::otherErr) {
    response_code = -2;
  }
  response->setStatusCode(response_code);
  request->setTransferedBytes(resourceMan.send);

  curl_slist_free_all(list);
  curl_easy_cleanup(curl);
  return response;
}
