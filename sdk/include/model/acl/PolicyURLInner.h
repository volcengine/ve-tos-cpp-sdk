#pragma once

#include <string>
#include <utility>
#include <vector>
#include "PolicySignatureConditionInner.h"

namespace VolcengineTos {
class PolicyURLInner {
public:
    explicit PolicyURLInner(std::vector<PolicySignatureConditionInner> conditions)
            : conditions_(std::move(conditions)) {
    }
    PolicyURLInner() = default;
    virtual ~PolicyURLInner() = default;
    const std::vector<PolicySignatureConditionInner>& getConditions() const {
        return conditions_;
    }
    void setConditions(const std::vector<PolicySignatureConditionInner>& conditions) {
        conditions_ = conditions;
    }
    std::string toJsonString() const;

private:
    std::vector<PolicySignatureConditionInner> conditions_;
};
}  // namespace VolcengineTos
