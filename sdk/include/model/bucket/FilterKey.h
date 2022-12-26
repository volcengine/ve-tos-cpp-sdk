#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "FilterRule.h"

namespace VolcengineTos {
class FilterKey {
public:
    FilterKey() = default;
    virtual ~FilterKey() = default;
    explicit FilterKey(std::vector<FilterRule> rules) : rules_(std::move(rules)) {
    }
    const std::vector<FilterRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<FilterRule>& rules) {
        rules_ = rules;
    }

private:
    std::vector<FilterRule> rules_;
};
}  // namespace VolcengineTos
