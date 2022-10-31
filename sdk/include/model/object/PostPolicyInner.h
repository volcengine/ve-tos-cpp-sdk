#pragma once

#include <string>
#include <vector>
#include "PostSignatureConditionInner.h"
#include "../src/external/json/json.hpp"

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
    std::string toJsonString() const {
        nlohmann::json j;
        if (!expiration_.empty()) {
            j["expiration"] = expiration_;
        }
        nlohmann::json jsonConditons = nlohmann::json::array();
        for (auto& condition : conditions_) {
            nlohmann::json tempJson;
            if (condition.getAnOperator() != nullptr) {
                if (*condition.getAnOperator() == "content-length-range") {
                    nlohmann::json arrayJson;
                    arrayJson.emplace_back(*condition.getAnOperator());
                    arrayJson.emplace_back(stoll(condition.getKey()));
                    arrayJson.emplace_back(stoll(condition.getValue()));
                    jsonConditons.emplace_back(arrayJson);
                } else {
                    std::vector<std::string> temp = {*condition.getAnOperator(), condition.getKey(),
                                                     condition.getValue()};
                    nlohmann::json arrayJson(temp);
                    jsonConditons.emplace_back(arrayJson);
                }
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
    std::vector<PostSignatureConditionInner> conditions_;
    std::string expiration_;
};
}  // namespace VolcengineTos
