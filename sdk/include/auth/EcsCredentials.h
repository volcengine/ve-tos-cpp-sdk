#pragma once

#include "Credentials.h"
#include <mutex>
#include <utility>
#include <atomic>
#include <chrono>
#include "EcsToken.h"
#include "transport/Transport.h"
namespace VolcengineTos {
class EcsCredentials : public Credentials {
public:
    EcsCredentials();
    ~EcsCredentials() override = default;
    explicit EcsCredentials(std::string role);
    EcsCredentials(std::string role, std::string url);
    EcsCredentials(const EcsCredentials& ec) {
        transport_ = ec.transport_;
        credential_ = ec.credential_;
        role_ = ec.role_;
    }
    EcsCredentials& operator=(const EcsCredentials& ec) {
        transport_ = ec.transport_;
        credential_ = ec.credential_;
        role_ = ec.role_;
        return *this;
    }

    Credential credential() override;

    const std::string& getRole() const {
        return role_;
    }
    void setRole(const std::string& role) {
        role_ = role;
    }
    const std::string& getUrl() const {
        return url_;
    }
    void setUrl(const std::string& url) {
        url_ = url;
    }

private:
    void updateCredential();
    void reload();
    Credential credential_;
    std::mutex update_;
    std::atomic<long long> expiration_ = {};  // todo 放到 cacheToken 中
    std::shared_ptr<Transport> transport_;
    std::string role_;
    std::string url_;
};
}  // namespace VolcengineTos
