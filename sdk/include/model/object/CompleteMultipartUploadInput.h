#pragma once

#include <string>
#include <utility>
#include <vector>
#include <ostream>
#include <algorithm>
#include "InnerUploadedPart.h"
#include "UploadPartOutput.h"
#include "UploadPartCopyOutput.h"
namespace VolcengineTos {
namespace inner {
class InnerCompleteMultipartUploadInput {
public:
    std::string toJsonString();
    explicit InnerCompleteMultipartUploadInput(int length) {
        parts_.resize(length);
    }
    void setPartsByIdx(const InnerUploadedPart& part, int i) {
        if (i < parts_.size()) {
            parts_[i] = part;
        }
    }
    const std::vector<InnerUploadedPart>& getParts() const {
        return parts_;
    }
    void setParts(const std::vector<InnerUploadedPart>& parts) {
        parts_ = parts;
    }
    void sort() {
        std::sort(parts_.begin(), parts_.end(), compareByPartNumber);
    }

private:
    static bool compareByPartNumber(const InnerUploadedPart& partA, const InnerUploadedPart& partB) {
        return partA.getPartNumber() < partB.getPartNumber();
    }
    std::vector<InnerUploadedPart> parts_;
};
}  // namespace inner
class [[gnu::deprecated]] CompleteMultipartUploadInput {
public:
    CompleteMultipartUploadInput(std::string key, std::string uploadId, std::vector<UploadPartOutput> uploadedParts)
            : key_(std::move(key)), uploadID_(std::move(uploadId)), uploadedParts_(std::move(uploadedParts)) {
    }

    int getUploadedPartsLength() {
        return uploadedParts_.size();
    }

    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }
    const std::vector<UploadPartOutput>& getUploadedParts() const {
        return uploadedParts_;
    }
    void setUploadedParts(const std::vector<UploadPartOutput>& uploadedParts) {
        uploadedParts_ = uploadedParts;
    }

private:
    std::string key_;
    std::string uploadID_;
    std::vector<UploadPartOutput> uploadedParts_;
};

class [[gnu::deprecated]] CompleteMultipartCopyUploadInput {
public:
    CompleteMultipartCopyUploadInput(std::string key, std::string uploadId,
                                     std::vector<UploadPartCopyOutput> uploadedParts)
            : key_(std::move(key)), uploadID_(std::move(uploadId)), uploadedParts_(std::move(uploadedParts)) {
    }

    int getUploadedPartsLength() {
        return uploadedParts_.size();
    }

    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }
    const std::vector<UploadPartCopyOutput>& getUploadedParts() const {
        return uploadedParts_;
    }
    void setUploadedParts(const std::vector<UploadPartCopyOutput>& uploadedParts) {
        uploadedParts_ = uploadedParts;
    }

private:
    std::string key_;
    std::string uploadID_;
    std::vector<UploadPartCopyOutput> uploadedParts_;
};
}  // namespace VolcengineTos
