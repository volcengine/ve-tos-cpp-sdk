#pragma once

#include <string>
#include <Type.h>
#include <utility>
#include <vector>
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteBucketCustomDomainInput : public GenericInput {
public:
    DeleteBucketCustomDomainInput(std::string bucket, std::string domain)
            : bucket_(std::move(bucket)), domain_(std::move(domain)) {
    }
    DeleteBucketCustomDomainInput() = default;
    virtual ~DeleteBucketCustomDomainInput() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getDomain() const {
        return domain_;
    }
    void setDomain(const std::string& domain) {
        domain_ = domain;
    }

private:
    std::string bucket_;
    std::string domain_;
};
}  // namespace VolcengineTos
