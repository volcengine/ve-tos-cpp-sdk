#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "../src/external/json/json.hpp"
#include "RealTimeLogConfiguration.h"

namespace VolcengineTos {
class GetBucketRealTimeLogOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const RealTimeLogConfiguration& getConfiguration() const {
        return configuration_;
    }
    void setConfiguration(const RealTimeLogConfiguration& configuration) {
        configuration_ = configuration;
    }

    void fromJsonString(const std::string& input) {
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

private:
    RequestInfo requestInfo_;
    RealTimeLogConfiguration configuration_;
};
}  // namespace VolcengineTos
