#pragma once

#include <memory>
#include <string>

#include "HttpRequest.h"
#include "utils/BaseUtils.h"

namespace VolcengineTos {
class HttpResponse {
public:
    HttpResponse();

    ~HttpResponse() = default;

    void setHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    void removeHeader(const std::string& key) {
        headers_.erase(key);
    }

    bool hasHeader(const std::string& key) {
        return headers_.find(key) != headers_.end();
    }

    std::string getHeaderValueByKey(const std::string& key) {
        return MapUtils::findValueByKeyIgnoreCase(headers_, key);
    }

    const std::map<std::string, std::string>& Headers() {
        return headers_;
    }

    void setBody(const std::shared_ptr<std::iostream>& body) {
        body_ = body;
    }

    std::shared_ptr<std::iostream>& Body() {
        return body_;
    }

    void setStatusCode(int code) {
        statusCode_ = code;
    }

    int statusCode() const {
        return statusCode_;
    }

    void setStatusMsg(const std::string& msg) {
        statusMsg_ = msg;
    }

    std::string statusMsg() {
        return statusMsg_;
    }

    void setStatus(const int& status) {
        status_ = status;
    }

    int status() {
        return status_;
    }

    const std::string& getId2() const {
        return Id2_;
    }
    void setId2(const std::string& id2) {
        Id2_ = id2;
    }

    size_t getBodySize();

    uint64_t getHashCrc64Result() const {
        return hashCrc64Result;
    }
    void setHashCrc64Result(uint64_t hashcrc64result) {
        hashCrc64Result = hashcrc64result;
    }
private:
    int status_;  // succ, refused, otherErr
    mutable int statusCode_;
    mutable std::string Id2_;
    mutable std::string statusMsg_;
    std::map<std::string, std::string> headers_;
    std::shared_ptr<std::iostream> body_;
    size_t bodySize_;
    uint64_t hashCrc64Result = 0;
};
}  // namespace VolcengineTos
