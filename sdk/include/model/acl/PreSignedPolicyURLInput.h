#pragma once

#include <string>
#include <utility>
#include <vector>
#include "Type.h"
#include "PolicySignatureCondition.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class PreSignedPolicyURLInput : public GenericInput {
public:
    PreSignedPolicyURLInput(std::string bucket, int64_t expires, std::vector<PolicySignatureCondition> conditions,
                            std::string alternativeEndpoint)
            : bucket_(std::move(bucket)),
              expires_(expires),
              conditions_(std::move(conditions)),
              alternativeEndpoint_(std::move(alternativeEndpoint)) {
    }
    PreSignedPolicyURLInput(std::string bucket, int64_t expires, std::vector<PolicySignatureCondition> conditions)
            : bucket_(std::move(bucket)), expires_(expires), conditions_(std::move(conditions)) {
    }
    PreSignedPolicyURLInput() = default;
    virtual ~PreSignedPolicyURLInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    int64_t getExpires() const {
        return expires_;
    }
    void setExpires(int64_t expires) {
        expires_ = expires;
    }
    const std::vector<PolicySignatureCondition>& getConditions() const {
        return conditions_;
    }
    void setConditions(const std::vector<PolicySignatureCondition>& conditions) {
        conditions_ = conditions;
    }
    const std::string& getAlternativeEndpoint() const {
        return alternativeEndpoint_;
    }
    void setAlternativeEndpoint(const std::string& alternativeEndpoint) {
        alternativeEndpoint_ = alternativeEndpoint;
    }
    bool isCustomDomain() const {
        return isCustomDomain_;
    }
    void setIsCustomDomain(bool isCustomDomain) {
        isCustomDomain_ = isCustomDomain;
    }

private:
    std::string bucket_;
    int64_t expires_ = 0;
    std::vector<PolicySignatureCondition> conditions_;
    std::string alternativeEndpoint_;
    bool isCustomDomain_ = false;
};
}  // namespace VolcengineTos
