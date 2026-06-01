#pragma once
#include "BucketBase.h"

#include <string>
namespace VolcengineTos {

class BucketBaseInput : public BucketBase {
public:
    BucketBaseInput() = default;
    ~BucketBaseInput() override = default;
    explicit BucketBaseInput(const std::string& bucket) : BucketBase(bucket) {
    }

    void setIsCustomDomain(const bool isCustomDomain) {
        isCustomDomain_ = isCustomDomain;
    }

    bool isCustomDomain() const {
        return isCustomDomain_;
    }

protected:
    std::string valid() override {
        std::string error_string;
        if (isCustomDomain()) {
            return "";
        }
        return BucketBase::valid();
    }

private:
    bool isCustomDomain_ = false;
};

}  // namespace VolcengineTos