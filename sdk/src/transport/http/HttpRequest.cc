#include "transport/http/HttpRequest.h"
#include <memory>
#include <utility>
#include <iostream>

using namespace VolcengineTos;
HttpRequest::HttpRequest(const std::string& method,
                         const std::shared_ptr<std::iostream>& responseBody)
    : method_(method), url_(), transferedBytes_(), headers_(),
      responseOutput_(responseBody){
}

HttpRequest::~HttpRequest() = default;