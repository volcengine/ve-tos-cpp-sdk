#include "model/bucket/PutBucketRealTimeLogInput.h"
#include "../src/external/json/json.hpp"

using namespace nlohmann;

std::string VolcengineTos::PutBucketRealTimeLogInput::toJsonString() const {
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
