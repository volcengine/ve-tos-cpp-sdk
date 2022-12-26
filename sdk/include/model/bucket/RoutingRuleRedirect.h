#pragma once

#include <string>
#include <utility>
#include "Type.h"
namespace VolcengineTos {
class RoutingRuleRedirect {
public:
    RoutingRuleRedirect() = default;
    virtual ~RoutingRuleRedirect() = default;
    RoutingRuleRedirect(ProtocolType protocolType, std::string hostName, std::string replaceKeyPrefixWith,
                        std::string replaceKeyWith, int httpRedirectCode)
            : protocolType_(protocolType),
              hostName_(std::move(hostName)),
              replaceKeyPrefixWith_(std::move(replaceKeyPrefixWith)),
              replaceKeyWith_(std::move(replaceKeyWith)),
              httpRedirectCode_(httpRedirectCode) {
    }
    RoutingRuleRedirect(ProtocolType protocolType, const std::string& hostName, const std::string& replaceKeyPrefixWith,
                        int httpRedirectCode)
            : protocolType_(protocolType),
              hostName_(hostName),
              replaceKeyPrefixWith_(replaceKeyPrefixWith),
              httpRedirectCode_(httpRedirectCode) {
    }
    ProtocolType getProtocolType() const {
        return protocolType_;
    }
    void setProtocolType(ProtocolType protocolType) {
        protocolType_ = protocolType;
    }
    const std::string& getHostName() const {
        return hostName_;
    }
    void setHostName(const std::string& hostName) {
        hostName_ = hostName;
    }
    const std::string& getReplaceKeyPrefixWith() const {
        return replaceKeyPrefixWith_;
    }
    void setReplaceKeyPrefixWith(const std::string& replaceKeyPrefixWith) {
        replaceKeyPrefixWith_ = replaceKeyPrefixWith;
    }
    const std::string& getReplaceKeyWith() const {
        return replaceKeyWith_;
    }
    void setReplaceKeyWith(const std::string& replaceKeyWith) {
        replaceKeyWith_ = replaceKeyWith;
    }
    int getHttpRedirectCode() const {
        return httpRedirectCode_;
    }
    void setHttpRedirectCode(int httpRedirectCode) {
        httpRedirectCode_ = httpRedirectCode;
    }

private:
    ProtocolType protocolType_ = ProtocolType::NotSet;
    std::string hostName_;
    std::string replaceKeyPrefixWith_;
    std::string replaceKeyWith_;
    int httpRedirectCode_ = 0;
};
}  // namespace VolcengineTos
