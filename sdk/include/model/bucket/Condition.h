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
        return keyPrefix;
    }
    void setKeyPrefix(const std::string& keyPrefix) {
        Condition::keyPrefix = keyPrefix;
    }
    const std::string& getKeySuffix() const {
        return keySuffix;
    }
    void setKeySuffix(const std::string& keySuffix) {
        Condition::keySuffix = keySuffix;
    }

private:
    int httpCode_ = 0;
    std::string keyPrefix;
    std::string keySuffix;
};
}  // namespace VolcengineTos
