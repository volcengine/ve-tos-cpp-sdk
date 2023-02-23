#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"
#include "../src/external/json/json.hpp"
#include "ReplicationRule.h"

namespace VolcengineTos {
class GetBucketReplicationOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::vector<ReplicationRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<ReplicationRule>& rules) {
        rules_ = rules;
    }

    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);
        if (j.contains("Rules")) {
            nlohmann::json rules = j.at("Rules");
            ReplicationRule rule_;
            for (auto& r : rules) {
                if (r.contains("ID")) {
                    rule_.setId(r.at("ID").get<std::string>());
                }
                if (r.contains("Status")) {
                    std::string status = r.at("Status").get<std::string>();
                    rule_.setStatus(StringtoStatusType[status]);
                }
                if (r.contains("PrefixSet")) {
                    rule_.setPrefixSet(r.at("PrefixSet").get<std::vector<std::string>>());
                }
                if (r.contains("HistoricalObjectReplication")) {
                    std::string historicalObjectReplication = r.at("HistoricalObjectReplication").get<std::string>();
                    rule_.setHistoricalObjectReplication(StringtoStatusType[historicalObjectReplication]);
                }
                if (r.contains("Destination")) {
                    auto destination = r.at("Destination");
                    Destination destination_;
                    if (destination.contains("Bucket")) {
                        destination_.setBucket(destination.at("Bucket").get<std::string>());
                    }
                    if (destination.contains("Location")) {
                        destination_.setLocation(destination.at("Location").get<std::string>());
                    }
                    if (destination.contains("StorageClass")) {
                        destination_.setStorageClass(
                                StringtoStorageClassType[destination.at("StorageClass").get<std::string>()]);
                    }
                    if (destination.contains("StorageClassInheritDirective")) {
                        destination_.setStorageClassInheritDirective(
                                StringtoStorageClassInheritDirectiveType[destination.at("StorageClassInheritDirective")
                                                                                 .get<std::string>()]);
                    }
                    rule_.setDestination(destination_);
                }
                if (r.contains("Progress")) {
                    auto progress = r.at("Progress");
                    Progress progress_;
                    if (progress.contains("HistoricalObject")) {
                        progress_.setHistoricalObject(progress.at("HistoricalObject").get<double>());
                    }
                    if (progress.contains("NewObject")) {
                        progress_.setNewObject(progress.at("NewObject").get<std::string>());
                    }
                    rule_.setProgress(std::make_shared<Progress>(progress_));
                }
                rules_.emplace_back(rule_);
            }
        }
    }

private:
    RequestInfo requestInfo_;
    std::vector<ReplicationRule> rules_;
};
}  // namespace VolcengineTos
