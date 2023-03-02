#include "model/bucket/PutBucketReplicationInput.h"
#include "../src/external/json/json.hpp"

std::string VolcengineTos::PutBucketReplicationInput::toJsonString() const {
    nlohmann::json j;
    nlohmann::json ruleArray = nlohmann::json::array();
    for (auto& r : rules_) {
        nlohmann::json rule;
        if (!r.getId().empty()) {
            rule["ID"] = r.getId();
        }
        if (r.getStatus() != StatusType::NotSet) {
            rule["Status"] = StatusTypetoString[r.getStatus()];
        }
        if (!r.getPrefixSet().empty()) {
            nlohmann::json prefixSet(r.getPrefixSet());
            rule["PrefixSet"] = prefixSet;
        }
        const auto& destination_ = r.getDestination();
        nlohmann::json destination;
        if (!destination_.getBucket().empty()) {
            destination["Bucket"] = destination_.getBucket();
        }
        if (!destination_.getLocation().empty()) {
            destination["Location"] = destination_.getLocation();
        }
        if (destination_.getStorageClass() != StorageClassType::NotSet) {
            destination["StorageClass"] = StorageClassTypetoString[destination_.getStorageClass()];
        }
        if (destination_.getStorageClassInheritDirective() != StorageClassInheritDirectiveType::NotSet) {
            destination["StorageClassInheritDirective"] =
                    StorageClassInheritDirectiveTypetoString[destination_.getStorageClassInheritDirective()];
        }
        if (!destination.empty()) {
            rule["Destination"] = destination;
        }
        if (r.getHistoricalObjectReplication() != StatusType::NotSet) {
            rule["HistoricalObjectReplication"] = StatusTypetoString[r.getHistoricalObjectReplication()];
        }
        if (r.getProgress() != nullptr) {
            auto progress_ = *r.getProgress();
            nlohmann::json progress;
            if (progress_.getHistoricalObject() != 0) {
                progress["HistoricalObject"] = progress_.getHistoricalObject();
            }
            if (!progress_.getNewObject().empty()) {
                progress["NewObject"] = progress_.getNewObject();
            }
            rule["Progress"] = progress;
        }
        ruleArray.push_back(rule);
    }
    if (!ruleArray.empty())
        j["Rules"] = ruleArray;
    if (!role_.empty()) {
        j["Role"] = role_;
    }
    return j.dump();
}
