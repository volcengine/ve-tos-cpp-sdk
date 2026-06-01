#pragma once

#include <string>

#include "Type.h"

namespace VolcengineTos {
class SetObjectTimeInput {
public:
    SetObjectTimeInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    SetObjectTimeInput() = default;
    ~SetObjectTimeInput() = default;

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
    timespec getModifyTimestamp() const {
        return modifyTimestamp_;
    }
    void setModifyTimestamp(timespec modifyTimestamp) {
        modifyTimestamp_ = modifyTimestamp;
    }

private:
    std::string bucket_;
    std::string key_;
    timespec modifyTimestamp_{0, 0};
};
}  // namespace VolcengineTos
