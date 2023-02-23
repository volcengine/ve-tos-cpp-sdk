#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "model/RequestInfo.h"

namespace VolcengineTos {
class GetBucketLifecycleOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::vector<LifecycleRule>& getRules() const {
        return rules_;
    }
    void setRules(const std::vector<LifecycleRule>& rules) {
        rules_ = rules;
    }

    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);
        if (j.contains("Rules")) {
            nlohmann::json rules = j.at("Rules");
            LifecycleRule rule_;
            for (auto& r : rules) {
                if (r.contains("ID")) {
                    rule_.setId(r.at("ID").get<std::string>());
                }
                if (r.contains("Prefix")) {
                    rule_.setPrefix(r.at("Prefix").get<std::string>());
                }
                if (r.contains("Status")) {
                    auto prefix = r.at("Status").get<std::string>();
                    rule_.setStatus(StringtoStatusType[prefix]);
                }
                if (r.contains("Transitions")) {
                    auto trans = r.at("Transitions");
                    std::vector<Transition> transitions_;
                    for (auto& t : trans) {
                        Transition transition_;
                        if (t.contains("Date")) {
                            transition_.setDate(
                                    TimeUtils::transLastModifiedStringToTime(t.at("Date").get<std::string>()));
                        }
                        if (t.contains("Days")) {
                            transition_.setDays(t.at("Days").get<int>());
                        }
                        if (t.contains("StorageClass")) {
                            transition_.setStorageClass(
                                    StringtoStorageClassType[t.at("StorageClass").get<std::string>()]);
                        }
                        transitions_.emplace_back(transition_);
                    }
                    rule_.setTransitions(transitions_);
                }
                if (r.contains("Expiration")) {
                    auto ex = r.at("Expiration");
                    Expiration expiration_;
                    if (ex.contains("Date")) {
                        expiration_.setDate(TimeUtils::transLastModifiedStringToTime(ex.at("Date").get<std::string>()));
                    }
                    if (ex.contains("Days")) {
                        expiration_.setDays(ex.at("Days").get<int>());
                    }
                    rule_.setExpiratioon(std::make_shared<Expiration>(expiration_));
                }
                if (r.contains("NoncurrentVersionTransitions")) {
                    auto ncvt = r.at("NoncurrentVersionTransitions");
                    std::vector<NoncurrentVersionTransition> noncurrentVersionTransitions_;
                    for (auto& t : ncvt) {
                        NoncurrentVersionTransition noncurrentVersionTransition_;
                        if (t.contains("NoncurrentDays")) {
                            noncurrentVersionTransition_.setNoncurrentDays(t.at("NoncurrentDays").get<int>());
                        }
                        if (t.contains("StorageClass")) {
                            noncurrentVersionTransition_.setStorageClass(
                                    StringtoStorageClassType[t.at("StorageClass").get<std::string>()]);
                        }
                        noncurrentVersionTransitions_.emplace_back(noncurrentVersionTransition_);
                    }
                    rule_.setNoncurrentVersionTransitions(noncurrentVersionTransitions_);
                }
                if (r.contains("NoncurrentVersionExpiration")) {
                    auto ncve = r.at("Expiration");
                    NoncurrentVersionExpiration expiration_;
                    if (ncve.contains("NoncurrentDays")) {
                        expiration_.setNoncurrentDays(ncve.at("NoncurrentDays").get<int>());
                    }
                    rule_.setNoncurrentVersionExpiration(std::make_shared<NoncurrentVersionExpiration>(expiration_));
                }
                if (r.contains("Tags")) {
                    auto tags = r.at("Tags");
                    std::vector<Tag> tags_;
                    for (auto& t : tags) {
                        Tag tag_;
                        if (t.contains("Key")) {
                            tag_.setKey(t.at("Key").get<std::string>());
                        }
                        if (t.contains("Value")) {
                            tag_.setValue(t.at("Value").get<std::string>());
                        }
                        tags_.emplace_back(tag_);
                    }
                    rule_.setTags(tags_);
                }
                if (r.contains("AbortIncompleteMultipartUpload")) {
                    auto aimu = r.at("AbortIncompleteMultipartUpload");
                    AbortInCompleteMultipartUpload abortInCompleteMultipartUpload_;
                    if (aimu.contains("DaysAfterInitiation")) {
                        abortInCompleteMultipartUpload_.setDaysAfterInitiation(
                                aimu.at("DaysAfterInitiation").get<int>());
                    }
                    rule_.setAbortInCompleteMultipartUpload(
                            std::make_shared<AbortInCompleteMultipartUpload>(abortInCompleteMultipartUpload_));
                }
                rules_.emplace_back(rule_);
            }
        }
    }

private:
    RequestInfo requestInfo_;
    std::vector<LifecycleRule> rules_;
};
}  // namespace VolcengineTos
