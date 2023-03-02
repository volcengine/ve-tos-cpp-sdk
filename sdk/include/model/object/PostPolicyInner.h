#pragma once

#include <string>
#include <vector>
#include "PostSignatureConditionInner.h"

namespace VolcengineTos {
class PostPolicyInner {
public:
    PostPolicyInner(std::vector<PostSignatureConditionInner> conditions, std::string expiration)
            : conditions_(std::move(conditions)), expiration_(std::move(expiration)) {
    }
    const std::vector<PostSignatureConditionInner>& getConditions() const {
        return conditions_;
    }
    void setConditions(const std::vector<PostSignatureConditionInner>& conditions) {
        conditions_ = conditions;
    }
    const std::string& getExpiration() const {
        return expiration_;
    }
    void setExpiration(const std::string& expiration) {
        expiration_ = expiration;
    }
    std::string toJsonString() const;

private:
    std::vector<PostSignatureConditionInner> conditions_;
    std::string expiration_;
};
}  // namespace VolcengineTos
