#pragma once
#include "Type.h"
#include "model/async/base/BaseHttp.h"
#include "model/async/base/Queries.h"

namespace VolcengineTos {
class ListBucketsAsyncInput final : public BaseHttp, public Queries {
    friend class TosAsyncClient;

public:
    ListBucketsAsyncInput() = default;
    explicit ListBucketsAsyncInput(BucketType bucketType) : bucketType_(bucketType) {
    }
    ~ListBucketsAsyncInput() override = default;
    void setBucketType(BucketType bucketType) {
        bucketType_ = bucketType;
    }
    BucketType getBucketType() const {
        return bucketType_;
    }

protected:
    void input2Headers() override {
        BaseHttp::input2Headers();
        addHeader(HEADER_BUCKET_TYPE, BucketTypetoString[bucketType_]);
    }

    void headers2Output(HttpResponse& response) override {
    }

private:
    BucketType bucketType_ = BucketType::FNS;
};
}  // namespace VolcengineTos
