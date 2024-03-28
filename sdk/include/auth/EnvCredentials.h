#pragma once

#include "Credentials.h"

#include <utility>
namespace VolcengineTos {
class EnvCredentials : public Credentials {
public:
    EnvCredentials() = default;
    ~EnvCredentials() override = default;

    Credential credential() override {
        auto value = std::getenv("TOS_ACCESS_KEY");
        Credential cred(accessKey_, secretKey_, securityToken_);

        if (value) {
            cred.setAccessKeyId(value);

            value = std::getenv("TOS_SECRET_KEY");

            if (value) {
                cred.setAccessKeySecret(value);
            }

            value = std::getenv("TOS_SECURITY_TOKEN");

            if (value) {
                cred.setSecurityToken(value);
            }
        }

        return {cred};
    }

    const std::string& getAccessKey() const {
        return accessKey_;
    }
    void setAccessKey(const std::string& accessKey) {
        accessKey_ = accessKey;
    }
    const std::string& getSecretKey() const {
        return secretKey_;
    }
    void setSecretKey(const std::string& secretKey) {
        secretKey_ = secretKey;
    }
    const std::string& getSecurityToken() const {
        return securityToken_;
    }
    void setSecurityToken(const std::string& securityToken) {
        securityToken_ = securityToken;
    }

private:
    std::string accessKey_;
    std::string secretKey_;
    std::string securityToken_;
};
}  // namespace VolcengineTos
