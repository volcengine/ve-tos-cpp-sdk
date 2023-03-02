#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "GrantV2.h"
#include "Owner.h"
namespace VolcengineTos {
class GetBucketAclOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const Owner& getOwner() const {
        return owner_;
    }
    void setOwner(const Owner& owner) {
        owner_ = owner;
    }
    const std::vector<GrantV2>& getGrant() const {
        return grant_;
    }
    void setGrant(const std::vector<GrantV2>& grant) {
        grant_ = grant;
    }

    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    Owner owner_;
    std::vector<GrantV2> grant_;
};
}  // namespace VolcengineTos
