#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Transition.h"
#include "Expiration.h"
#include "NoncurrentVersionTransition.h"
#include "NoncurrentVersionExpiration.h"
#include "Tag.h"
#include "AbortInCompleteMultipartUpload.h"

namespace VolcengineTos {
class LifecycleRule {
public:
    LifecycleRule() = default;
    LifecycleRule(std::string id, std::string prefix, StatusType status, std::vector<Transition> transitions,
                  std::shared_ptr<Expiration> expiration,
                  std::vector<NoncurrentVersionTransition> noncurrentVersionTransitions,
                  std::shared_ptr<NoncurrentVersionExpiration> noncurrentVersionExpiration, std::vector<Tag> tags,
                  std::shared_ptr<AbortInCompleteMultipartUpload> abortInCompleteMultipartUpload)
            : id_(std::move(id)),
              prefix_(std::move(prefix)),
              status_(status),
              transitions_(std::move(transitions)),
              expiration_(std::move(expiration)),
              noncurrentVersionTransitions_(std::move(noncurrentVersionTransitions)),
              noncurrentVersionExpiration_(std::move(noncurrentVersionExpiration)),
              tags_(std::move(tags)),
              abortInCompleteMultipartUpload_(std::move(abortInCompleteMultipartUpload)) {
    }
    virtual ~LifecycleRule() = default;
    const std::string& getId() const {
        return id_;
    }
    void setId(const std::string& id) {
        id_ = id;
    }
    const std::string& getPrefix() const {
        return prefix_;
    }
    void setPrefix(const std::string& prefix) {
        prefix_ = prefix;
    }
    StatusType getStatus() const {
        return status_;
    }
    const std::string& getStringFormatStatus() const {
        return StatusTypetoString[status_];
    }
    void setStatus(StatusType status) {
        status_ = status;
    }
    const std::vector<Transition>& getTransitions() const {
        return transitions_;
    }
    void setTransitions(const std::vector<Transition>& transitions) {
        transitions_ = transitions;
    }
    const std::shared_ptr<Expiration>& getExpiratioon() const {
        return expiration_;
    }
    void setExpiratioon(const std::shared_ptr<Expiration>& expiratioon) {
        expiration_ = expiratioon;
    }
    void setExpiratioon(time_t date) {
        expiration_ = std::make_shared<Expiration>(date);
    }
    void setExpiratioon(int days) {
        expiration_ = std::make_shared<Expiration>(days);
    }
    const std::vector<NoncurrentVersionTransition>& getNoncurrentVersionTransitions() const {
        return noncurrentVersionTransitions_;
    }
    void setNoncurrentVersionTransitions(const std::vector<NoncurrentVersionTransition>& noncurrentVersionTransitions) {
        noncurrentVersionTransitions_ = noncurrentVersionTransitions;
    }
    const std::shared_ptr<NoncurrentVersionExpiration>& getNoncurrentVersionExpiration() const {
        return noncurrentVersionExpiration_;
    }
    void setNoncurrentVersionExpiration(
            const std::shared_ptr<NoncurrentVersionExpiration>& noncurrentVersionExpiration) {
        noncurrentVersionExpiration_ = noncurrentVersionExpiration;
    }
    void setNoncurrentVersionExpiration(int noncurrentDays) {
        noncurrentVersionExpiration_ = std::make_shared<NoncurrentVersionExpiration>(noncurrentDays);
    }
    const std::vector<Tag>& getTags() const {
        return tags_;
    }
    void setTags(const std::vector<Tag>& tags) {
        tags_ = tags;
    }

    const std::shared_ptr<AbortInCompleteMultipartUpload>& getAbortInCompleteMultipartUpload() const {
        return abortInCompleteMultipartUpload_;
    }
    void setAbortInCompleteMultipartUpload(
            const std::shared_ptr<AbortInCompleteMultipartUpload>& abortInCompleteMultipartUpload) {
        abortInCompleteMultipartUpload_ = abortInCompleteMultipartUpload;
    }
    bool hasExpiration() const {
        if (expiration_ != nullptr) {
            return true;
        }
        return false;
    }
    bool hasNoncurrentVersionExpiration() const {
        if (noncurrentVersionExpiration_ != nullptr) {
            return true;
        }
        return false;
    }
    bool hasAbortInCompleteMultipartUpload() const {
        if (abortInCompleteMultipartUpload_ != nullptr) {
            return true;
        }
        return false;
    }
    void addTag(const Tag& tag) {
        tags_.push_back(tag);
    }
    void addTransition(const Transition& transition) {
        transitions_.push_back(transition);
    }
    void addNoncurrentVersionTransition(const NoncurrentVersionTransition& noncurrentVersionTransition) {
        noncurrentVersionTransitions_.push_back(noncurrentVersionTransition);
    }

private:
    std::string id_;
    std::string prefix_;
    StatusType status_ = StatusType::NotSet;
    std::vector<Transition> transitions_;
    std::shared_ptr<Expiration> expiration_;
    std::vector<NoncurrentVersionTransition> noncurrentVersionTransitions_;
    std::shared_ptr<NoncurrentVersionExpiration> noncurrentVersionExpiration_;
    std::vector<Tag> tags_;
    std::shared_ptr<AbortInCompleteMultipartUpload> abortInCompleteMultipartUpload_;
};
}  // namespace VolcengineTos
