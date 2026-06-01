#pragma once

#include "common/Common.h"
#include "model/async/base/BucketNameBase.h"
#include "model/async/base/Headers.h"
#include "model/async/base/KeyBase.h"
#include "transport/http/HttpResponse.h"
#include "utils/BaseUtils.h"

#include <string>
#include <utility>

namespace VolcengineTos {
class CopySourceBase : virtual public Headers {
public:
    CopySourceBase() = default;
    ~CopySourceBase() override = default;
    CopySourceBase(std::string srcBucket, std::string srcKey)
            : sourceBucket_(std::move(srcBucket)), sourceKey_(std::move(srcKey)) {
    }

    void setSourceBucket(const std::string& sourceBucket) {
        sourceBucket_ = sourceBucket;
    }
    void setSourceKey(const std::string& sourceKey) {
        sourceKey_ = sourceKey;
    }
    const std::string& getSourceVersionId() const {
        return sourceVersionID_;
    }
    void setSourceVersionId(const std::string& sourceVersionId) {
        sourceVersionID_ = sourceVersionId;
    }

protected:
    std::string valid() override {
        std::string error_string = isValidKey(sourceKey_);
        if (!error_string.empty()) {
            return error_string;
        }

        error_string = isValidBucketName(sourceBucket_);
        if (!error_string.empty()) {
            return error_string;
        }

        return "";
    }

    void input2Headers() override {
        addHeader(HEADER_COPY_SOURCE, copySource(sourceBucket_, sourceKey_, sourceVersionID_));
    }

    void headers2Output(HttpResponse& response) override {
        sourceVersionID_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_COPY_SOURCE_VERSION_ID);
    }

private:
    std::string sourceBucket_;
    std::string sourceKey_;
    std::string sourceVersionID_;

    static std::string copySource(const std::string& bucket, const std::string& object_, const std::string& versionID) {
        std::string ret;
        const auto object = StringUtils::uriEncode(object_, false);
        if (versionID.empty()) {
            return ret.append("/").append(bucket).append("/").append(object);
        }
        return ret.append("/").append(bucket).append("/").append(object).append("?versionId=").append(versionID);
    }
};
}  // namespace VolcengineTos
