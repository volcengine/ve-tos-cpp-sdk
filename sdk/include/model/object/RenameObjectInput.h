#pragma once

#include <string>
#include <utility>
#include "Type.h"
namespace VolcengineTos {
class RenameObjectInput {
public:
    RenameObjectInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    RenameObjectInput(std::string bucket, std::string key, std::string newKey)
            : bucket_(std::move(bucket)), key_(std::move(key)), newKey_(std::move(newKey)) {
    }
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
    const std::string& getNewKey() const {
        return newKey_;
    }
    void setNewKey(const std::string& newKey) {
        newKey_ = newKey;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string newKey_;
};
}  // namespace VolcengineTos
