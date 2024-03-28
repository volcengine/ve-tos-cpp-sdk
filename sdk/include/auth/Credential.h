#pragma once
#include <string>
namespace VolcengineTos {
class Credential {
public:
    Credential() = default;
    ~Credential() = default;
    Credential(std::string accessKeyId, std::string accessKeySecret, std::string securityToken)
            : accessKeyId_(std::move(accessKeyId)),
              accessKeySecret_(std::move(accessKeySecret)),
              securityToken_(std::move(securityToken)) {
    }
    const std::string& getAccessKeyId() const {
        return accessKeyId_;
    }
    void setAccessKeyId(const std::string& accessKeyId) {
        Credential::accessKeyId_ = accessKeyId;
    }
    const std::string& getAccessKeySecret() const {
        return accessKeySecret_;
    }
    void setAccessKeySecret(const std::string& accessKeySecret) {
        Credential::accessKeySecret_ = accessKeySecret;
    }
    const std::string& getSecurityToken() const {
        return securityToken_;
    }
    void setSecurityToken(const std::string& securityToken) {
        Credential::securityToken_ = securityToken;
    }

private:
    std::string accessKeyId_;
    std::string accessKeySecret_;
    std::string securityToken_;
};
}  // namespace VolcengineTos
