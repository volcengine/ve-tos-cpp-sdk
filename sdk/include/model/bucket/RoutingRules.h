#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "RoutingRule.h"
namespace VolcengineTos {
class RoutingRules {
public:
    explicit RoutingRules(std::vector<RoutingRule> rules) : rules_(std::move(rules)) {
    }
    RoutingRules() = default;
    virtual ~RoutingRules() = default;
    const std::vector<RoutingRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<RoutingRule>& rules) {
        rules_ = rules;
    }

private:
    std::vector<RoutingRule> rules_;
};
}  // namespace VolcengineTos
