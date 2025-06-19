#pragma once

#include <string>
#include "Type.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteObjectInput : public GenericInput {
public:
    DeleteObjectInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    DeleteObjectInput() = default;
    ~DeleteObjectInput() = default;
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
    const std::string& getVersionID() const {
        return versionID_;
    }
    void setVersionID(const std::string& versionID) {
        versionID_ = versionID;
    }

    bool getRecursive() const {
        return recursive_;
    }

    void setRecursive(bool isRecursive) {
        recursive_ = isRecursive;
    }

    bool getSkipTrash() const {
        return skipTrash_;
    }

    void setSkipTrash(bool skipTrash) {
        skipTrash_ = skipTrash;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string versionID_;
    bool recursive_ = false;
    bool skipTrash_ = false;
};
}  // namespace VolcengineTos
