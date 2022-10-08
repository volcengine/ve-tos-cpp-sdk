#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class [[gnu::deprecated]] CreateBucketOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getLocation() const {
        return location_;
    }
    void setLocation(const std::string& location) {
        location_ = location;
    }

private:
    RequestInfo requestInfo_;
    std::string location_;
};
}  // namespace VolcengineTos
