#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "model/RequestInfo.h"

namespace VolcengineTos {
class TosError {
public:
    void fromJsonString(const std::string& error);

    std::string String() const {
        std::ostringstream ss;
        if (isClientError_) {
            ss << "TosClientError: {message=" << message_ << "}";
        } else {
            ss << "TosServerError: {statusCode=" << statusCode_ << ", ";
            ss << "code=" << code_ << ", ";
            ss << "message=" << message_ << ", ";
            ss << "requestID=" << requestID_ << ", ";
            ss << "hostID=" << hostID_ << "}";
            ss << "EC=" << EC_ << "}";
            ss << "requestUrl=" << requestUrl_ << "}";
        }
        return ss.str();
    }

    int getStatusCode() const {
        return statusCode_;
    }
    void setStatusCode(int statusCode) {
        statusCode_ = statusCode;
    }
    const std::string& getCode() const {
        return code_;
    }
    void setCode(const std::string& code) {
        code_ = code;
    }
    const std::string& getMessage() const {
        return message_;
    }
    void setMessage(const std::string& message) {
        message_ = message;
    }
    const std::string& getCondition() const {
        return condition_;
    }
    void setCondition(const std::string& condition) {
        condition_ = condition;
    }
    const std::string& getRequestId() const {
        return requestID_;
    }
    void setRequestId(const std::string& requestId) {
        requestID_ = requestId;
    }
    const std::string& getHostId() const {
        return hostID_;
    }
    void setHostId(const std::string& hostId) {
        hostID_ = hostId;
    }
    bool isClientError() const {
        return isClientError_;
    }
    void setIsClientError(bool isClientError) {
        isClientError_ = isClientError;
    }
    bool IsTosHandlerError() const {
        return isTosHandlerError_;
    }
    void setIsTosHandlerError(bool isTosHandlerError) {
        isTosHandlerError_ = isTosHandlerError;
    }
    const std::string& getResource() const {
        return resource_;
    }
    void setResource(const std::string& resource) {
        resource_ = resource;
    }
    int getCurlErrCode() const {
        return curlErrCode_;
    }
    void setCurlErrCode(int curlErrCode) {
        curlErrCode_ = curlErrCode;
    }
    const std::string& getRequestUrl() const {
        return requestUrl_;
    }
    void setRequestUrl(const std::string& requestUrl) {
        requestUrl_ = requestUrl;
    }
    const std::string& getEc() const {
        return EC_;
    }
    void setEc(const std::string& ec) {
        EC_ = ec;
    }
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }

private:
    bool isClientError_ = false;
    bool isTosHandlerError_ = false;
    int statusCode_ = -1;
    std::string code_;
    std::string message_;
    std::string requestID_;
    std::string hostID_;
    std::string resource_;
    std::string condition_;
    RequestInfo requestInfo_;
    int curlErrCode_ = 0;
    std::string requestUrl_;
    std::string EC_;
};

}  // namespace VolcengineTos
