#pragma once

#include <string>
#include <utility>
#include "Type.h"
#include "RestoreJobParameters.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class RestoreObjectInput : public GenericInput {
public:
    RestoreObjectInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    RestoreObjectInput(const std::string& bucket, const std::string& key, int days,
                       const RestoreJobParameters& restoreJobParameters)
            : bucket_(bucket), key_(key), days_(days), restoreJobParameters_(restoreJobParameters) {
    }
    RestoreObjectInput(std::string bucket, std::string key, std::string versionId, int days,
                       const RestoreJobParameters& restoreJobParameters)
            : bucket_(std::move(bucket)),
              key_(std::move(key)),
              versionID_(std::move(versionId)),
              days_(days),
              restoreJobParameters_(restoreJobParameters) {
    }
    RestoreObjectInput() = default;
    ~RestoreObjectInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }
    int getDays() const {
        return days_;
    }
    void setDays(int days) {
        days_ = days;
    }
    const RestoreJobParameters& getRestoreJobParameters() const {
        return restoreJobParameters_;
    }
    void setRestoreJobParameters(const RestoreJobParameters& restoreJobParameters) {
        restoreJobParameters_ = restoreJobParameters;
    }
    const std::string& getNotificationCustomParameters() const {
        return notificationCustomParameters_;
    }
    void setNotificationCustomParameters(const std::string& notificationCustomParameters) {
        notificationCustomParameters_ = notificationCustomParameters;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    std::string key_;
    std::string versionID_;
    int days_ = 0;
    RestoreJobParameters restoreJobParameters_ = RestoreJobParameters(TierType::NotSet);
    std::string notificationCustomParameters_;
};
}  // namespace VolcengineTos
