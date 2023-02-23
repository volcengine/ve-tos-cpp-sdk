#pragma once

#include <string>
#include <vector>
#include "model/RequestInfo.h"
#include "ListedCommonPrefix.h"
#include "ListedUpload.h"
#include "../src/external/json/json.hpp"
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
    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);
        if (j.contains("Bucket"))
            j.at("Bucket").get_to(bucket_);
        if (j.contains("KeyMarker"))
            j.at("KeyMarker").get_to(keyMarker_);
        if (j.contains("UploadIDMarker"))
            j.at("UploadIDMarker").get_to(uploadIDMarker_);
        if (j.contains("MaxUploads"))
            j.at("MaxUploads").get_to(maxUploads_);
        if (j.contains("Prefix"))
            j.at("Prefix").get_to(prefix_);
        if (j.contains("Delimiter"))
            j.at("Delimiter").get_to(delimiter_);
        if (j.contains("EncodingType"))
            j.at("EncodingType").get_to(encodingType_);
        if (j.contains("IsTruncated"))
            j.at("IsTruncated").get_to(isTruncated_);
        if (j.contains("NextKeyMarker"))
            j.at("NextKeyMarker").get_to(nextKeyMarker_);
        if (j.contains("NextUploadIDMarker"))
            j.at("NextUploadIDMarker").get_to(nextUploadIdMarker_);

        if (j.contains("CommonPrefixes")) {
            nlohmann::json commonPrefixes = j.at("CommonPrefixes");
            for (auto& commonPrefixe : commonPrefixes) {
                ListedCommonPrefix lc;
                if (commonPrefixe.contains("Prefix"))
                    lc.setPrefix(commonPrefixe.at("Prefix").get<std::string>());
                commonPrefixes_.emplace_back(lc);
            }
        }
        if (j.contains("Uploads")) {
            nlohmann::json uploads = j.at("Uploads");
            for (auto& upload : uploads) {
                ListedUpload lu;
                if (upload.contains("Key"))
                    lu.setKey(upload.at("Key").get<std::string>());
                if (upload.contains("UploadId"))
                    lu.setUploadId(upload.at("UploadId").get<std::string>());
                if (upload.contains("StorageClass"))
                    lu.setStorageClass(StringtoStorageClassType[upload.at("StorageClass").get<std::string>()]);
                if (upload.contains("Initiated"))
                    lu.setInitiated(
                            TimeUtils::transLastModifiedStringToTime(upload.at("Initiated").get<std::string>()));
                if (upload.contains("Owner")) {
                    Owner owner;
                    if (upload.at("Owner").contains("ID")) {
                        owner.setId(upload.at("Owner").at("ID").get<std::string>());
                    }
                    if (upload.at("Owner").contains("DisplayName")) {
                        owner.setDisplayName(upload.at("Owner").at("DisplayName").get<std::string>());
                    }
                    lu.setOwner(owner);
                }
                uploads_.emplace_back(lu);
            }
        }
    }

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
