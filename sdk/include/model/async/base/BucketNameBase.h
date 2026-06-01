#pragma once
#include "PiplineInputInterface.h"
#include "PiplineOutputInterface.h"

#include <string>
#include <utility>
namespace VolcengineTos {
static std::string isValidBucketName(const std::string& name);
class BucketNameBase : virtual public PiplineInputInterface, virtual public PiplineOutputInterface {
    friend class TosAsyncClient;

public:
    BucketNameBase() = default;
    ~BucketNameBase() override = default;

    explicit BucketNameBase(std::string bucket) : bucket_(std::move(bucket)) {
    }

    void setBucketName(std::string& bucketName) {
        bucket_ = std::move(bucketName);
    }

    const std::string& getBucketName() const {
        return bucket_;
    }

protected:
    std::string valid() override {
        return isValidBucketName(bucket_);
    }

    void json2Output(json& j) override {
        if (j.contains("Bucket")) {
            bucket_ = j["Bucket"].get<std::string>();
        }
    }

private:
    std::string bucket_;
};

static std::string isValidBucketName(const std::string& name) {
    if (name.empty() || name.length() < 3 || name.length() > 63) {
        return "invalid bucket name, the length must be [3, 63]";
    }
    for (char c : name) {
        if (!(('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || c == '-')) {
            return "invalid bucket name, the character set is illegal";
        }
    }
    if (name[0] == '-' || name[name.length() - 1] == '-') {
        return "invalid bucket name, the bucket name can be neither starting with ' - ' nor ending with ' - '";
    }
    return "";
}
}  // namespace VolcengineTos