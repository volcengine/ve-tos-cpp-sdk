#pragma once

#include "model/RequestInfo.h"
#include "Type.h"
namespace VolcengineTos {
class GetBucketVersioningOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    VersioningStatusType getStatus() const {
        return status_;
    }
    void setStatus(VersioningStatusType status) {
        status_ = status;
    }

    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);
        if (j.contains("Status")) {
            status_ = StringtoVersioningStatusType[j.at("Status").get<std::string>()];
        }
    }
    const std::string& getStringFormatVersioningStatus() const {
        return VersioningStatusTypetoString[status_];
    }

private:
    RequestInfo requestInfo_;
    VersioningStatusType status_ = VersioningStatusType::NotSet;
};
}  // namespace VolcengineTos
