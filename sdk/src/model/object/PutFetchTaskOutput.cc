#include "../src/external/json/json.hpp"

#include "model/object/PutFetchTaskOutput.h"

void VolcengineTos::PutFetchTaskOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("TaskId"))
        j.at("TaskId").get_to(taskID_);
}
