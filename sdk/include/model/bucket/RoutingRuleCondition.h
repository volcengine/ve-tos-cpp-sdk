#pragma once

#include <string>
#include <utility>
namespace VolcengineTos {
class RoutingRuleCondition {
public:
    RoutingRuleCondition() = default;
    virtual ~RoutingRuleCondition() = default;
    RoutingRuleCondition(std::string keyPrefixEquals, int httpErrorCodeReturnedEquals)
            : keyPrefixEquals_(std::move(keyPrefixEquals)), httpErrorCodeReturnedEquals_(httpErrorCodeReturnedEquals) {
    }
    const std::string& getKeyPrefixEquals() const {
        return keyPrefixEquals_;
    }
    void setKeyPrefixEquals(const std::string& keyPrefixEquals) {
        keyPrefixEquals_ = keyPrefixEquals;
    }
    int getHttpErrorCodeReturnedEquals() const {
        return httpErrorCodeReturnedEquals_;
    }
    void setHttpErrorCodeReturnedEquals(int httpErrorCodeReturnedEquals) {
        httpErrorCodeReturnedEquals_ = httpErrorCodeReturnedEquals;
    }

private:
    std::string keyPrefixEquals_;
    int httpErrorCodeReturnedEquals_ = 0;
};
}  // namespace VolcengineTos
