#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "../src/external/json/json.hpp"
#include "RealTimeLogConfiguration.h"

namespace VolcengineTos {
class PutBucketRealTimeLogInput {
public:
    explicit PutBucketRealTimeLogInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketRealTimeLogInput(std::string bucket, const RealTimeLogConfiguration& configuration)
            : bucket_(std::move(bucket)), configuration_(configuration) {
    }
    PutBucketRealTimeLogInput() = default;
    virtual ~PutBucketRealTimeLogInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const RealTimeLogConfiguration& getConfiguration() const {
        return configuration_;
    }
    void setConfiguration(const RealTimeLogConfiguration& configuration) {
        configuration_ = configuration;
    }

    std::string toJsonString() const {
        nlohmann::json j;
        nlohmann::json configuration;
        if (!configuration_.getRole().empty()) {
            configuration["Role"] = configuration_.getRole();
        }
        nlohmann::json conf;
        auto conf_ = configuration_.getConfiguration();
        if (conf_.isUseServiceTopic()) {
            conf["UseServiceTopic"] = conf_.isUseServiceTopic();
        }
        if (!conf_.getTlsProjectId().empty()) {
            conf["TLSProjectID"] = conf_.getTlsProjectId();
        }
        if (!conf_.getTlsTopicId().empty()) {
            conf["TLSTopicID"] = conf_.getTlsTopicId();
        }
        if (!conf.empty()) {
            configuration["AccessLogConfiguration"] = conf;
        }

        if (!configuration.empty()) {
            j["RealTimeLogConfiguration"] = configuration;
        }
        return j.dump();
    }

private:
    std::string bucket_;
    RealTimeLogConfiguration configuration_;
};
}  // namespace VolcengineTos
