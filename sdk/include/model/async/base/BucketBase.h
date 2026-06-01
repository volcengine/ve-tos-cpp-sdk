#pragma once

#include "BucketNameBase.h"
#include "Headers.h"
#include "Type.h"
#include "common/Common.h"
#include "utils/BaseUtils.h"

#include <utility>

namespace VolcengineTos {

class BucketBase : public BucketNameBase, virtual public Headers {
public:
    BucketBase() = default;
    ~BucketBase() override = default;
    explicit BucketBase(std::string bucket) : BucketNameBase(std::move(bucket)) {
    }

    BucketType getBucketType() const {
        return bucketType_;
    }
    void setBucketType(BucketType bucketType) {
        bucketType_ = bucketType;
    }

protected:
    void input2Headers() override {
        addHeader(HEADER_BUCKET_TYPE, BucketTypetoString[bucketType_]);
    }

    void headers2Output(HttpResponse& response) override {
        bucketType_ = StringtoBucketType[MapUtils::findValueByKeyIgnoreCase(response.Headers(), HEADER_BUCKET_TYPE)];
    }

private:
    BucketType bucketType_ = BucketType::FNS;
};

}  // namespace VolcengineTos
