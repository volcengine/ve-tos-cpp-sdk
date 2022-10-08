#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "Grant.h"
#include "Owner.h"
namespace VolcengineTos {
class [[gnu::deprecated]] GetObjectAclOutput {
public:
    void fromJsonString(const std::string& input);
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getVersionId() const {
        return versionId_;
    }
    void setVersionId(const std::string& versionId) {
        versionId_ = versionId;
    }
    const Owner& getOwner() const {
        return owner_;
    }
    void setOwner(const Owner& owner) {
        owner_ = owner;
    }
    const std::vector<Grant>& getGrant() const {
        return grant_;
    }
    void setGrant(const std::vector<Grant>& grant) {
        grant_ = grant;
    }

private:
    RequestInfo requestInfo_;
    std::string versionId_;
    Owner owner_;
    std::vector<Grant> grant_;
};
}  // namespace VolcengineTos
