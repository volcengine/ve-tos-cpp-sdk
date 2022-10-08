#pragma once

#include <ctime>
#include "Credential.h"
namespace VolcengineTos {
class FederationToken {
public:
    const Credential& getCredential() const {
        return credential_;
    }
    void setCredential(const Credential& credential) {
        credential_ = credential;
    }
    time_t getExpiration() const {
        return expiration_;
    }
    void setExpiration(time_t expiration) {
        expiration_ = expiration;
    }

private:
    Credential credential_;
    time_t expiration_{};
};
}  // namespace VolcengineTos
