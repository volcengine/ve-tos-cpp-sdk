#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class [[gnu::deprecated]] UploadPartCopyInput : public GenericInput {
public:
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }
    const std::string& getDestinationKey() const {
        return destinationKey_;
    }
    void setDestinationKey(const std::string& destinationKey) {
        destinationKey_ = destinationKey;
    }
    const std::string& getSourceBucket() const {
        return sourceBucket_;
    }
    void setSourceBucket(const std::string& sourceBucket) {
        sourceBucket_ = sourceBucket;
    }
    const std::string& getSourceKey() const {
        return sourceKey_;
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
    int64_t getStartOffset() const {
        return startOffset_;
    }
    void setStartOffset(int64_t startOffset) {
        startOffset_ = startOffset;
    }
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partSize) {
        partSize_ = partSize;
    }
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partNumber) {
        partNumber_ = partNumber;
    }

private:
    std::string uploadID_;
    std::string destinationKey_;
    std::string sourceBucket_;
    std::string sourceKey_;
    std::string sourceVersionID_;
    int64_t startOffset_;
    int64_t partSize_;
    int partNumber_;
};
}  // namespace VolcengineTos
