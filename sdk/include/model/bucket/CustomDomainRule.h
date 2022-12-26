#pragma once

#include <string>
#include <utility>
#include "Type.h"

namespace VolcengineTos {
class CustomDomainRule {
public:
    CustomDomainRule() = default;
    virtual ~CustomDomainRule() = default;
    CustomDomainRule(std::string certId, CertStatusType certStatus, std::string domain, std::string cname,
                     bool forbidden, std::string forbiddenReason)
            : certId_(std::move(certId)),
              certStatus_(certStatus),
              domain_(std::move(domain)),
              cname_(std::move(cname)),
              forbidden_(forbidden),
              forbiddenReason_(std::move(forbiddenReason)) {
    }
    explicit CustomDomainRule(std::string domain) : domain_(std::move(domain)) {
    }
    const std::string& getCertId() const {
        return certId_;
    }
    void setCertId(const std::string& certId) {
        certId_ = certId;
    }
    CertStatusType getCertStatus() const {
        return certStatus_;
    }
    void setCertStatus(CertStatusType certStatus) {
        certStatus_ = certStatus;
    }
    const std::string& getDomain() const {
        return domain_;
    }
    void setDomain(const std::string& domain) {
        domain_ = domain;
    }
    const std::string& getCname() const {
        return cname_;
    }
    void setCname(const std::string& cname) {
        cname_ = cname;
    }
    bool isForbidden() const {
        return forbidden_;
    }
    void setForbidden(bool forbidden) {
        forbidden_ = forbidden;
    }
    const std::string& getForbiddenReason() const {
        return forbiddenReason_;
    }
    void setForbiddenReason(const std::string& forbiddenReason) {
        forbiddenReason_ = forbiddenReason;
    }

private:
    std::string certId_;
    CertStatusType certStatus_ = CertStatusType::NotSet;
    std::string domain_;
    std::string cname_;
    bool forbidden_ = false;
    std::string forbiddenReason_;
};
}  // namespace VolcengineTos
