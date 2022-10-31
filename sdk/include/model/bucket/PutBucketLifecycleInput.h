#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>
#include "MirrorBackRule.h"
#include "LifecycleRule.h"

namespace VolcengineTos {
class PutBucketLifecycleInput {
public:
    explicit PutBucketLifecycleInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketLifecycleInput(std::string bucket, std::vector<LifecycleRule> rules)
            : bucket_(std::move(bucket)), rules_(std::move(rules)) {
    }
    PutBucketLifecycleInput() = default;
    virtual ~PutBucketLifecycleInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::vector<LifecycleRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<LifecycleRule>& rules) {
        rules_ = rules;
    }

    std::string toJsonString() const {
        nlohmann::json j;
        nlohmann::json ruleArray = nlohmann::json::array();
        for (auto& r : rules_) {
            nlohmann::json rule;
            if (!r.getId().empty()) {
                rule["ID"] = r.getId();
            }
            if (!r.getPrefix().empty()) {
                rule["Prefix"] = r.getPrefix();
            }
            if (r.getStatus() != StatusType::NotSet) {
                rule["Status"] = StatusTypetoString[r.getStatus()];
            }
            if (!r.getTransitions().empty()) {
                nlohmann::json transitionsArray = nlohmann::json::array();
                for (auto& t : r.getTransitions()) {
                    nlohmann::json transitions;
                    if (t.getDate() != 0) {
                        auto data = t.getDate();
                        std::string Date = TimeUtils::transLastModifiedTimeToString(data);
                        transitions["Date"] = Date;
                    }
                    if (t.getDays() != 0) {
                        transitions["Days"] = t.getDays();
                    }
                    if (t.getStorageClass() != StorageClassType::NotSet) {
                        transitions["StorageClass"] = StorageClassTypetoString[t.getStorageClass()];
                    }
                    transitionsArray.push_back(transitions);
                }
                if (!transitionsArray.empty())
                    rule["Transitions"] = transitionsArray;
            }
            if (r.getExpiratioon() != nullptr) {
                auto expiration_ = *r.getExpiratioon();
                nlohmann::json expiration;
                if (expiration_.getDate() != 0) {
                    auto data = expiration_.getDate();
                    std::string date = TimeUtils::transLastModifiedTimeToString(data);
                    expiration["Date"] = date;
                }
                if (expiration_.getDays() != 0) {
                    expiration["Days"] = expiration_.getDays();
                }
                rule["Expiration"] = expiration;
            }
            if (!r.getNoncurrentVersionTransitions().empty()) {
                nlohmann::json ncvTransitionsArray = nlohmann::json::array();
                for (auto& n : r.getNoncurrentVersionTransitions()) {
                    nlohmann::json nTransitions;
                    if (n.getNoncurrentDays() != 0) {
                        nTransitions["NoncurrentDays"] = n.getNoncurrentDays();
                    }
                    if (n.getStorageClass() != StorageClassType::NotSet) {
                        nTransitions["StorageClass"] = StorageClassTypetoString[n.getStorageClass()];
                    }
                    ncvTransitionsArray.push_back(nTransitions);
                }
                if (!ncvTransitionsArray.empty())
                    rule["NoncurrentVersionTransitions"] = ncvTransitionsArray;
            }
            if (r.getNoncurrentVersionExpiration() != nullptr) {
                auto ncvExpiration_ = *r.getNoncurrentVersionExpiration();
                nlohmann::json ncvExpiration;
                if (ncvExpiration_.getNoncurrentDays() != 0) {
                    ncvExpiration["NoncurrentDays"] = ncvExpiration_.getNoncurrentDays();
                }
                rule["NoncurrentVersionExpiration"] = ncvExpiration;
            }
            if (!r.getTags().empty()) {
                nlohmann::json tagArray = nlohmann::json::array();
                for (auto& t : r.getTags()) {
                    nlohmann::json tag;
                    if (!t.getKey().empty()) {
                        tag["Key"] = t.getKey();
                    }
                    if (!t.getValue().empty()) {
                        tag["Value"] = t.getValue();
                    }
                    tagArray.push_back(tag);
                }
                if (!tagArray.empty())
                    rule["Tags"] = tagArray;
            }
            if (r.getAbortInCompleteMultipartUpload() != nullptr) {
                auto abort_ = *r.getAbortInCompleteMultipartUpload();
                nlohmann::json abort;
                if (abort_.getDaysAfterInitiation() != 0) {
                    abort["DaysAfterInitiation"] = abort_.getDaysAfterInitiation();
                }
                rule["AbortIncompleteMultipartUpload"] = abort;
            }
            ruleArray.push_back(rule);
        }
        if (!ruleArray.empty())
            j["Rules"] = ruleArray;
        return j.dump();
    }

private:
    std::string bucket_;
    std::vector<LifecycleRule> rules_;
};
}  // namespace VolcengineTos
