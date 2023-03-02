#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "Owner.h"
#include "GrantV2.h"
#include "model/object/PostSignatureCondition.h"
#include "model/object/ContentLengthRange.h"
namespace VolcengineTos {
class PreSignedPostSignatureInput {
public:
    PreSignedPostSignatureInput() = default;
    PreSignedPostSignatureInput(std::string bucket, std::string key)
            : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    PreSignedPostSignatureInput(std::string bucket, std::string key, int64_t expires)
            : bucket_(std::move(bucket)), key_(std::move(key)), expires_(expires) {
    }
    PreSignedPostSignatureInput(std::string bucket, std::string key, int64_t expires,
                                std::vector<PostSignatureCondition> conditions,
                                std::shared_ptr<ContentLengthRange> contentLengthRange)
            : bucket_(std::move(bucket)),
              key_(std::move(key)),
              expires_(expires),
              conditions_(std::move(conditions)),
              contentLengthRange_(std::move(contentLengthRange)) {
    }
    virtual ~PreSignedPostSignatureInput() = default;
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
    int64_t getExpires() const {
        return expires_;
    }
    void setExpires(int64_t expires) {
        expires_ = expires;
    }
    const std::vector<PostSignatureCondition>& getConditions() const {
        return conditions_;
    }
    void setConditions(const std::vector<PostSignatureCondition>& conditions) {
        conditions_ = conditions;
    }
    const std::shared_ptr<ContentLengthRange>& getContentLengthRange() const {
        return contentLengthRange_;
    }
    void setContentLengthRange(const std::shared_ptr<ContentLengthRange>& contentLengthRange) {
        contentLengthRange_ = contentLengthRange;
    }

private:
    std::string bucket_;
    std::string key_;
    int64_t expires_ = 0;

    std::vector<PostSignatureCondition> conditions_;
    std::shared_ptr<ContentLengthRange> contentLengthRange_;
};
}  // namespace VolcengineTos
