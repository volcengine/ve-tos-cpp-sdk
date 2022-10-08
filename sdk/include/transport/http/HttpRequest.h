#pragma once

#include "common/Common.h"
#include "Url.h"
#include "Type.h"
#include <memory>
#include <sstream>
#include <string>

namespace VolcengineTos {

class HttpRequest {
public:
    HttpRequest(const std::string& method = "GET",
                const std::shared_ptr<std::iostream>& responseBody = std::make_shared<std::stringstream>());
    ~HttpRequest();

    void setHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }
    void setHeaders(const std::map<std::string, std::string>& headers) {
        headers_ = headers;
    }
    void removeHeader(const std::string& key) {
        headers_.erase(key);
    }
    bool hasHeader(const std::string& key) const {
        return headers_.find(key) != headers_.end();
    }
    std::string getHeaderValueByKey(const std::string& key) const {
        auto it = headers_.find(key);
        if (it != headers_.end())
            return it->second;
        else
            return {};
    }
    const std::map<std::string, std::string>& Headers() const {
        return headers_;
    }

    std::shared_ptr<std::iostream>& Body() {
        return body_;
    }
    void setBody(const std::shared_ptr<std::iostream>& body) {
        body_ = body;
    }

    const std::string& method() {
        return method_;
    }
    void setMethod(const std::string& method) {
        method_ = method;
    }

    Url url() const {
        return url_;
    }
    void setUrl(const Url& url) {
        url_ = url;
    }

    void setTransferedBytes(int64_t value) {
        transferedBytes_ = value;
    }
    uint64_t TransferedBytes() const {
        return transferedBytes_;
    }

    std::shared_ptr<std::iostream> responseOutput() {
        return responseOutput_;
    }

    int64_t getContentLength() const {
        return contentLength_;
    }
    void setContentLength(int64_t contentLength) {
        contentLength_ = contentLength;
    }
    const DataTransferListener& getDataTransferListener() const {
        return dataTransferListener_;
    }
    void setDataTransferListener(const DataTransferListener& datatransferlistener) {
        dataTransferListener_ = datatransferlistener;
    }
    bool isCheckCrc64() const {
        return checkCrc64;
    }
    void setCheckCrc64(bool checkcrc64) {
        checkCrc64 = checkcrc64;
    }
    const std::shared_ptr<RateLimiter>& getRateLimiter() const {
        return rateLimiter_;
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& rateLimiter) {
        rateLimiter_ = rateLimiter;
    }
    uint64_t getPreHashCrc64Ecma() const {
        return preHashCrc64ecma_;
    }
    void setPreHashCrc64Ecma(uint64_t prehashcrc64ecma) {
        preHashCrc64ecma_ = prehashcrc64ecma;
    }

private:
    std::string method_;
    Url url_;
    int64_t transferedBytes_;
    int64_t contentLength_ = 0;
    std::map<std::string, std::string> headers_;
    std::shared_ptr<std::iostream> body_;
    std::shared_ptr<std::iostream> responseOutput_;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;
    bool checkCrc64 = false;
    uint64_t preHashCrc64ecma_ = 0;
};
}  // namespace VolcengineTos
