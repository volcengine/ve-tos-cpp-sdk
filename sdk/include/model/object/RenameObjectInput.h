#pragma once

#include <string>
#include <utility>
#include "Type.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class RenameObjectInput : public GenericInput {
public:
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

    bool getRecursiveMkdir() const {
        return recursiveMkdir_;
    }

    void setRecursiveMkdir(bool recursiveMkdir) {
        recursiveMkdir_ = recursiveMkdir;
    }

    bool getForbidOverwrite() const {
        return forbidOverwrite_;
    }

    void setForbidOverwrite(bool forbidOverwrite) {
        forbidOverwrite_ = forbidOverwrite;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string newKey_;
    bool recursiveMkdir_ = false;
    bool forbidOverwrite_ = false;
};
}  // namespace VolcengineTos
