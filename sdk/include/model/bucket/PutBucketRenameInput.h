#pragma once
#include <string>
#include <utility>
namespace VolcengineTos {
class PutBucketRenameInput {
public:
    explicit PutBucketRenameInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketRenameInput(std::string bucket, bool renameEnable)
            : bucket_(std::move(bucket)), renameEnable_(renameEnable) {
    }
    PutBucketRenameInput() = default;
    virtual ~PutBucketRenameInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    bool isRenameEnable() const {
        return renameEnable_;
    }
    void setRenameEnable(bool renameEnable) {
        renameEnable_ = renameEnable;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    bool renameEnable_{};
};
}  // namespace VolcengineTos
