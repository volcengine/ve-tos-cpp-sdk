#pragma once

#include <string>
#include <vector>
#include <sstream>
#include "model/RequestInfo.h"

namespace VolcengineTos {
class TosError {
public:
    void fromJsonString(const std::string& error);

    std::string String() {
        std::ostringstream ss;
        if (isClientError_) {
            ss << "TosClientError: {message=" << message_ << "}";
        } else {
            ss << "TosServerError: {statusCode=" << statusCode_ << ", ";
            ss << "code=" << code_ << ", ";
            ss << "message=" << message_ << ", ";
            ss << "requestID=" << requestID_ << ", ";
            ss << "hostID=" << hostID_ << "}";
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
        isClientError = isClientError;
    }
    const std::string& getResource() const {
        return resource_;
    }
    void setResource(const std::string& resource) {
        resource_ = resource;
    }

private:
    bool isClientError_ = false;
    int statusCode_ = 400;
    std::string code_;
    std::string message_;
    std::string requestID_;
    std::string hostID_;
    std::string resource_;
    RequestInfo requestInfo_;
};

}  // namespace VolcengineTos
