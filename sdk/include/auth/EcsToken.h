#pragma once

#include <ctime>
#include "Credential.h"
namespace VolcengineTos {
class EcsToken {
public:
    const Credential& getCredential() const {
        return credential_;
    }
    void setCredential(const Credential& credential) {
        credential_ = credential;
    }

private:
    Credential credential_;
};
}  // namespace VolcengineTos
