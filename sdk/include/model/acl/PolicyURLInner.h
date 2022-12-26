#pragma once

#include <string>
#include <utility>
#include <vector>
#include "../src/external/json/json.hpp"
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
    std::string toJsonString() const {
        nlohmann::json j;
        nlohmann::json jsonConditons = nlohmann::json::array();
        for (auto& condition : conditions_) {
            nlohmann::json tempJson;
            if (condition.getAnOperator() != nullptr) {
                std::vector<std::string> temp = {*condition.getAnOperator(), condition.getKey(), condition.getValue()};
                nlohmann::json arrayJson(temp);
                jsonConditons.emplace_back(arrayJson);

            } else {
                tempJson[condition.getKey()] = condition.getValue();
                jsonConditons.emplace_back(tempJson);
            }
        }
        if (!jsonConditons.empty())
            j["conditions"] = jsonConditons;
        return j.dump();
    }

private:
    std::vector<PolicySignatureConditionInner> conditions_;
};
}  // namespace VolcengineTos
