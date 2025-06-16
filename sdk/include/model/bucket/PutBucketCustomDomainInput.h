#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>

#include "CustomDomainRule.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class PutBucketCustomDomainInput : public GenericInput {
public:
    explicit PutBucketCustomDomainInput(std::string bucket) : bucket_(std::move(bucket)) {
    }
    PutBucketCustomDomainInput(std::string bucket, const CustomDomainRule& rule)
            : bucket_(std::move(bucket)), rule_(rule) {
    }
    PutBucketCustomDomainInput() = default;
    virtual ~PutBucketCustomDomainInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const CustomDomainRule& getRules() const {
        return rule_;
    }
    void setRules(const CustomDomainRule& rules) {
        rule_ = rules;
    }

    std::string toJsonString() const;

private:
    std::string bucket_;
    CustomDomainRule rule_;
};
}  // namespace VolcengineTos
