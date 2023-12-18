#pragma once

#include <map>
#include <memory>
#include <utility>
#include "model/RequestInfo.h"
#include "common/Common.h"
#include "utils/BaseUtils.h"
namespace VolcengineTos {
class TosResponse {
public:
    explicit TosResponse(std::shared_ptr<std::iostream> content) : content_(std::move(content)) {
    }
    ~TosResponse() = default;
    std::string getRequestID();
    RequestInfo GetRequestInfo();

    int getStatusCode() const {
        return statusCode_;
    }
    void setStatusCode(int statusCode) {
        statusCode_ = statusCode;
    }
    const std::string& getStatusMsg() const {
        return statusMsg_;
    }
    void setStatusMsg(const std::string& statusMsg) {
        statusMsg_ = statusMsg;
    }
    int64_t getContentLength() const {
        return contentLength_;
    }
    void setContentLength(int64_t contentLength) {
        contentLength_ = contentLength;
    }

    std::string findHeader(const std::string& key) {
        return MapUtils::findValueByKeyIgnoreCase(headers_, key);
    }

    const std::map<std::string, std::string>& getHeaders() const {
        return headers_;
    }
    void setHeaders(const std::map<std::string, std::string>& headers) {
        headers_ = headers;
    }
    std::shared_ptr<std::iostream> getContent() const {
        return content_;
    }
    void setContent(const std::shared_ptr<std::iostream>& content) {
        content_ = content;
    }
    const std::string& getId2() const {
        return Id2_;
    }
    void setId2(const std::string& Id2) {
        Id2_ = Id2;
    }
    uint64_t getHashCrc64Result() const {
        return hashCrc64Result_;
    }
    void setHashCrc64Result(uint64_t hashCrc64Result) {
        hashCrc64Result_ = hashCrc64Result;
    }
    int getCurlErrCode() const {
        return curlErrCode_;
    }
    void setCurlErrCode(int curlErrCode) {
        curlErrCode_ = curlErrCode;
    }
    bool isHighLatency() const {
        return isHighLatency_;
    }
    void setIsHighLatency(bool isHighLatency) {
        isHighLatency_ = isHighLatency;
    }

private:
    int statusCode_{};
    std::string statusMsg_;
    int64_t contentLength_{};
    std::map<std::string, std::string> headers_;
    std::shared_ptr<std::iostream> content_;
    std::string Id2_;
    uint64_t hashCrc64Result_ = 0;
    int curlErrCode_ = 0;
    bool isHighLatency_ = false;
};
}  // namespace VolcengineTos
