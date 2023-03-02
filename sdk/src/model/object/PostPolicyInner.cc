#include "model/object/PostPolicyInner.h"
#include "../src/external/json/json.hpp"

std::string VolcengineTos::PostPolicyInner::toJsonString() const {
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
                std::vector<std::string> temp = {*condition.getAnOperator(), condition.getKey(), condition.getValue()};
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
