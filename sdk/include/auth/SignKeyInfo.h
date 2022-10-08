#pragma once

#include <string>
#include "Credential.h"
namespace VolcengineTos {
class SignKeyInfo {
public:
    SignKeyInfo(const std::string& date, const std::string& region, const Credential& credential)
            : date_(date), region_(region), credential_(credential) {
    }

    const std::string& getDate() const {
        return date_;
    }
    void setDate(const std::string& date) {
        date_ = date;
    }
    const std::string& getRegion() const {
        return region_;
    }
    void setRegion(const std::string& region) {
        region_ = region;
    }
    const Credential& getCredential() const {
        return credential_;
    }
    void setCredential(const Credential& credential) {
        credential_ = credential;
    }

private:
    std::string date_;
    std::string region_;
    Credential credential_;
};
}  // namespace VolcengineTos
