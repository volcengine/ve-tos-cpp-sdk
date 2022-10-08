
#pragma once

#include "model/RequestInfo.h"
#include "MirrorBackRule.h"
namespace VolcengineTos {
class GetBucketLocationOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getExtranetEndpoint() const {
        return extranetEndpoint_;
    }
    void setExtranetEndpoint(const std::string& extranetEndpoint) {
        extranetEndpoint_ = extranetEndpoint;
    }
    const std::string& getIntranetEndpoint() const {
        return intranetEndpoint_;
    }
    void setIntranetEndpoint(const std::string& intranetEndpoint) {
        intranetEndpoint_ = intranetEndpoint;
    }

private:
    RequestInfo requestInfo_;
    std::string extranetEndpoint_;
    std::string intranetEndpoint_;
};
}  // namespace VolcengineTos
