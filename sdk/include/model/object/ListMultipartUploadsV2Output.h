#pragma once

#include <string>
#include <vector>
#include "model/RequestInfo.h"
#include "ListedCommonPrefix.h"
#include "ListedUpload.h"

namespace VolcengineTos {
class ListMultipartUploadsV2Output {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getKeyMarker() const {
        return keyMarker_;
    }
    void setKeyMarker(const std::string& keymarker) {
        keyMarker_ = keymarker;
    }
    const std::string& getNextKeyMarker() const {
        return nextKeyMarker_;
    }
    void setNextKeyMarker(const std::string& nextkeymarker) {
        nextKeyMarker_ = nextkeymarker;
    }
    const std::string& getUploadIdMarker() const {
        return uploadIDMarker_;
    }
    void setUploadIdMarker(const std::string& uploadidmarker) {
        uploadIDMarker_ = uploadidmarker;
    }
    const std::string& getNextUploadIdMarker() const {
        return nextUploadIdMarker_;
    }
    void setNextUploadIdMarker(const std::string& nextuploadidmarker) {
        nextUploadIdMarker_ = nextuploadidmarker;
    }
    const std::string& getDelimiter() const {
        return delimiter_;
    }
    void setDelimiter(const std::string& delimiter) {
        delimiter_ = delimiter;
    }
    const std::string& getPrefix() const {
        return prefix_;
    }
    void setPrefix(const std::string& prefix) {
        prefix_ = prefix;
    }
    int getMaxUploads() const {
        return maxUploads_;
    }
    void setMaxUploads(int maxuploads) {
        maxUploads_ = maxuploads;
    }
    bool isTruncated() const {
        return isTruncated_;
    }
    void setIsTruncated(bool istruncated) {
        isTruncated_ = istruncated;
    }
    const std::vector<ListedUpload>& getUploads() const {
        return uploads_;
    }
    void setUploads(const std::vector<ListedUpload>& uploads) {
        uploads_ = uploads;
    }
    const std::vector<ListedCommonPrefix>& getCommonPrefixes() const {
        return commonPrefixes_;
    }
    void setCommonPrefixes(const std::vector<ListedCommonPrefix>& commonprefixes) {
        commonPrefixes_ = commonprefixes;
    }
    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::string bucket_;
    std::string prefix_;
    std::string keyMarker_;
    std::string uploadIDMarker_;
    int maxUploads_ = 0;
    std::string delimiter_;
    bool isTruncated_;
    std::string encodingType_;
    std::string nextKeyMarker_;
    std::string nextUploadIdMarker_;

    std::vector<ListedUpload> uploads_;
    std::vector<ListedCommonPrefix> commonPrefixes_;
};
}  // namespace VolcengineTos
