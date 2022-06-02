#pragma once

#include "Credentials.h"

#include <utility>
namespace VolcengineTos {
class StaticCredentials : public Credentials {
public:
  StaticCredentials() = default;
  ~StaticCredentials() override = default;
  StaticCredentials(std::string  accessKey, std::string secretKey)
      : accessKey_(std::move(accessKey)), secretKey_(std::move(secretKey)) {}
  StaticCredentials(std::string  accessKey, std::string secretKey, std::string securityToken)
      : accessKey_(std::move(accessKey)), secretKey_(std::move(secretKey)), securityToken_(std::move(securityToken)) {}

  Credential credential() override {
    Credential cred(accessKey_, secretKey_, securityToken_);
    return {cred};
  }

  const std::string &getAccessKey() const { return accessKey_; }
  void setAccessKey(const std::string &accessKey) { accessKey_ = accessKey; }
  const std::string &getSecretKey() const { return secretKey_; }
  void setSecretKey(const std::string &secretKey) { secretKey_ = secretKey; }
  const std::string &getSecurityToken() const { return securityToken_; }
  void setSecurityToken(const std::string &securityToken) {
    securityToken_ = securityToken;
  }

private:
  std::string accessKey_;
  std::string secretKey_;
  std::string securityToken_;
};
} // namespace VolcengineTos