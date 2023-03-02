#include "model/acl/PolicyURLInner.h"
#include "../src/external/json/json.hpp"

std::string VolcengineTos::PolicyURLInner::toJsonString() const {
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
