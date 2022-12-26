#pragma once

#include <string>
#include "Type.h"
#include "RoutingRuleCondition.h"
#include "RoutingRuleRedirect.h"
namespace VolcengineTos {
class RoutingRule {
public:
    RoutingRule(const RoutingRuleCondition& condition, const RoutingRuleRedirect& redirect)
            : condition_(condition), redirect_(redirect) {
    }
    RoutingRule() = default;
    virtual ~RoutingRule() = default;
    const RoutingRuleCondition& getCondition() const {
        return condition_;
    }
    void setCondition(const RoutingRuleCondition& condition) {
        condition_ = condition;
    }
    const RoutingRuleRedirect& getRedirect() const {
        return redirect_;
    }
    void setRedirect(const RoutingRuleRedirect& redirect) {
        redirect_ = redirect;
    }

private:
    RoutingRuleCondition condition_;
    RoutingRuleRedirect redirect_;
};
}  // namespace VolcengineTos
