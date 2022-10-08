#pragma once
#include <ctime>
#include <string>
#include <map>
namespace VolcengineTos {

class RequestInfo {
public:
    const std::string& getRequestId() const {
        return requestId_;
    }
    void setRequestId(const std::string& requestId) {
        requestId_ = requestId;
    }

    const std::string& getId2() const {
        return Id2_;
    }
    void setId2(const std::string& Id2) {
        Id2_ = Id2;
    }

    int getStatusCode() const {
        return statusCode_;
    }
    void setStatusCode(int statusCode) {
        statusCode_ = statusCode;
    }

    const std::map<std::string, std::string>& getHeaders() const {
        return headers_;
    }
    void setHeaders(const std::map<std::string, std::string>& headers) {
        headers_ = headers;
    }

private:
    std::string requestId_;
    std::string Id2_;
    int statusCode_;
    std::map<std::string, std::string> headers_;
};
}  // namespace VolcengineTos
