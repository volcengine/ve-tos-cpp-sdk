#include "model/bucket/GetBucketRealTimeLogOutput.h"
#include "../src/external/json/json.hpp"

void VolcengineTos::GetBucketRealTimeLogOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("RealTimeLogConfiguration")) {
        auto config = j.at("RealTimeLogConfiguration");
        if (config.contains("Role")) {
            configuration_.setRole(config.at("Role").get<std::string>());
        }
        if (config.contains("AccessLogConfiguration")) {
            auto acc = config.at("AccessLogConfiguration");
            AccessLogConfiguration config_;
            if (acc.contains("UseServiceTopic")) {
                config_.setUseServiceTopic(acc.at("UseServiceTopic").get<bool>());
            }
            if (acc.contains("TLSProjectID")) {
                config_.setTlsProjectId(acc.at("TLSProjectID").get<std::string>());
            }
            if (acc.contains("TLSTopicID")) {
                config_.setTlsTopicId(acc.at("TLSTopicID").get<std::string>());
            }
            configuration_.setConfiguration(config_);
        }
    }
}
