#pragma once

#include <string>
namespace VolcengineTos {
class Condition {
public:
    Condition() = default;
    virtual ~Condition() = default;
    explicit Condition(int httpCode) : httpCode_(httpCode) {
    }
    int getHttpCode() const {
        return httpCode_;
    }
    void setHttpCode(int httpCode) {
        httpCode_ = httpCode;
    }
    const std::string& getKeyPrefix() const {
        return keyPrefix_;
    }
    void setKeyPrefix(const std::string& keyPrefix) {
        keyPrefix_ = keyPrefix;
    }
    const std::string& getKeySuffix() const {
        return keySuffix_;
    }
    void setKeySuffix(const std::string& keySuffix) {
        keySuffix_ = keySuffix;
    }

private:
    int httpCode_ = 0;
    std::string keyPrefix_;
    std::string keySuffix_;
};
}  // namespace VolcengineTos
