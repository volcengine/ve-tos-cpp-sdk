#include "transport/http/HttpResponse.h"

using namespace VolcengineTos;

HttpResponse::HttpResponse()
        : status_(http::otherErr), statusCode_(-1), statusMsg_(), headers_(), Id2_(), body_(), bodySize_(0) {
}

size_t HttpResponse::getBodySize() {
    if (!body_) {
        return 0;
    }
    int64_t size = body_->tellp();
    return size;
}