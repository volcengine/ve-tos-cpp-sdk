#pragma once

#include "Type.h"
#include "common/Common.h"
#include "model/async/base/Headers.h"

#include <string>

namespace VolcengineTos {
class PermissionBase : virtual public Headers {
public:
    PermissionBase() = default;
    ~PermissionBase() override = default;

    ACLType getAcl() const {
        return acl_;
    }
    void setAcl(const ACLType acl) {
        acl_ = acl;
    }
    const std::string& getGrantFullControl() const {
        return grantFullControl_;
    }
    void setGrantFullControl(const std::string& grant_full_control) {
        grantFullControl_ = grant_full_control;
    }
    const std::string& getGrantRead() const {
        return grantRead_;
    }
    void setGrantRead(const std::string& value) {
        grantRead_ = value;
    }
    const std::string& getGrantReadAcp() const {
        return grantReadAcp_;
    }
    void setGrantReadAcp(const std::string& value) {
        grantReadAcp_ = value;
    }
    const std::string& getGrantWriteAcp() const {
        return grantWriteAcp_;
    }
    void setGrantWriteAcp(const std::string& value) {
        grantWriteAcp_ = value;
    }

protected:
    void input2Headers() override {
        addHeader(HEADER_ACL, ACLTypetoString[acl_]);

        addHeader(HEADER_GRANT_FULL_CONTROL, grantFullControl_);

        addHeader(HEADER_GRANT_READ, grantRead_);

        addHeader(HEADER_GRANT_READ_ACP, grantReadAcp_);

        addHeader(HEADER_GRANT_WRITE_ACP, grantWriteAcp_);
    }

private:
    ACLType acl_ = ACLType::NotSet;
    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWriteAcp_;
};
}  // namespace VolcengineTos
