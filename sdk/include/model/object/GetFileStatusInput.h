#pragma once

#include <string>

namespace VolcengineTos {
class GetFileStatusInput {
public:
    GetFileStatusInput() = default;
    ~GetFileStatusInput() = default;
    GetFileStatusInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
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

private:
    std::string bucket_;
    std::string key_;
};
}  // namespace VolcengineTos