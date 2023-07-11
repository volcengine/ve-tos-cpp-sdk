#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class GetBucketRenameOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    bool isRenameEnable() const {
        return renameEnable_;
    }
    void setRenameEnable(bool renameEnable) {
        renameEnable_ = renameEnable;
    }
    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    bool renameEnable_;
};
}  // namespace VolcengineTos
