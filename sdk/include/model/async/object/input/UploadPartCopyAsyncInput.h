#pragma once

#include <utility>

#include "model/async/object/input/UploadPartAsyncInput.h"
#include "model/async/object/base/CopyConditionBase.h"
#include "model/async/object/base/CopySourceBase.h"
#include "model/async/object/base/CopySourceRangeBase.h"
#include "model/async/object/base/CopySourceSseBase.h"

namespace VolcengineTos {
class UploadPartCopyAsyncInput final : public UploadPartAsyncInput,
                                       public CopyConditionBase,
                                       public CopySourceBase,
                                       public CopySourceRangeBase,
                                       public CopySourceSseBase {
    friend class TosAsyncClient;

public:
    UploadPartCopyAsyncInput() = default;
    ~UploadPartCopyAsyncInput() override = default;
    UploadPartCopyAsyncInput(const std::string& bucket, const std::string& key, const std::string& srcBucket,
                             const std::string& srcKey, const int& partNumber, const std::string& uploadId)
            : UploadPartAsyncInput(bucket, key, partNumber, uploadId, TransferEncoding::ContentLength),
              CopySourceBase(srcBucket, srcKey) {
    }

    const std::string& getDestinationKey() const {
        return getKey();
    }
    void setDestinationKey(const std::string& destinationKey) {
        return setKey(destinationKey);
    }

protected:
    std::string valid() override {
        std::string error_string = UploadPartAsyncInput::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        error_string = CopySourceBase::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        error_string = CopySourceRangeBase::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        error_string = CopySourceSseBase::valid();
        if (!error_string.empty()) {
            return error_string;
        }
        return "";
    }

    void input2Headers() override {
        UploadPartAsyncInput::input2Headers();
        CopyConditionBase::input2Headers();
        CopySourceBase::input2Headers();
        CopySourceRangeBase::input2Headers();
        CopySourceSseBase::input2Headers();
    }

    void headers2Output(HttpResponse& response) override {
    }
};
}  // namespace VolcengineTos
