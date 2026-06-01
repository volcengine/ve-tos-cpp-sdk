#pragma once
#include "BaseHttp.h"
#include "model/RequestInfo.h"

#include <string>
namespace VolcengineTos {

class BaseOutput : public BaseHttp {
public:
    BaseOutput() : statusCode_(0) {
    }
    ~BaseOutput() override = default;

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
    std::time_t getLastModified() const {
        return lastModified_;
    }
    void setLastModified(const std::time_t& lastModified) {
        lastModified_ = lastModified;
    }
    int getStatusCode() const {
        return statusCode_;
    }
    void setStatusCode(int statusCode) {
        statusCode_ = statusCode;
    }
    const std::string& getLocation() const {
        return location_;
    }
    void setLocation(const std::string& location) {
        location_ = location;
    }

    RequestInfo getRequestInfo() const {
        RequestInfo requestInfo;
        requestInfo.setStatusCode(statusCode_);
        requestInfo.setId2(Id2_);
        requestInfo.setRequestId(requestId_);
        requestInfo.setHeaders(getHeaders());
        return requestInfo;
    }

protected:
    void headers2Output(HttpResponse& response) override {
        BaseHttp::headers2Output(response);
        statusCode_ = response.statusCode();
        requestId_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_REQUEST_ID);
        Id2_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_ID_2);
        lastModified_ = TimeUtils::transGMTFormatStringToTime(
                MapUtils::findValueByKeyIgnoreCase(getHeaders(), http::HEADER_LAST_MODIFIED));
        location_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), http::HEADER_LOCATION);
    }

    void json2Output(json& j) override {
        if (j.contains("LastModified")) {
            lastModified_ = TimeUtils::transLastModifiedStringToTime(j["LastModified"].get<std::string>());
        }
    }

private:
    int statusCode_;
    std::string requestId_;
    std::string Id2_;
    std::time_t lastModified_ = 0;

    std::string location_;
};
}  // namespace VolcengineTos
