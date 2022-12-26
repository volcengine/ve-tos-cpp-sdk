#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "UploadFileInfo.h"
#include "UploadFilePartInfo.h"
#include "DownloadFileFileInfo.h"
#include "DownloadFileObjectInfo.h"
#include "DownloadFilePartInfo.h"
#include "../src/external/json/json.hpp"
#include "HeadObjectV2Output.h"
#include "HeadObjectV2Input.h"
#include "ResumableCopyCopySourceObjectInfo.h"
#include "ResumableCopyPartInfo.h"
namespace VolcengineTos {
class ResumableCopyCheckpoint {
public:
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partSize) {
        partSize_ = partSize;
    }
    const std::string& getCopySourceIfMatch() const {
        return copySourceIfMatch_;
    }
    void setCopySourceIfMatch(const std::string& copySourceIfMatch) {
        copySourceIfMatch_ = copySourceIfMatch;
    }
    const std::string& getCopySourceIfModifiedSince() const {
        return copySourceIfModifiedSince_;
    }
    void setCopySourceIfModifiedSince(const std::string& copySourceIfModifiedSince) {
        copySourceIfModifiedSince_ = copySourceIfModifiedSince;
    }
    const std::string& getCopySourceIfNoneMatch() const {
        return copySourceIfNoneMatch_;
    }
    void setCopySourceIfNoneMatch(const std::string& copySourceIfNoneMatch) {
        copySourceIfNoneMatch_ = copySourceIfNoneMatch;
    }
    const std::string& getCopySourceIfUnmodifiedSince() const {
        return copySourceIfUnmodifiedSince_;
    }
    void setCopySourceIfUnmodifiedSince(const std::string& copySourceIfUnmodifiedSince) {
        copySourceIfUnmodifiedSince_ = copySourceIfUnmodifiedSince;
    }
    const std::string& getCopySourceSsecAlgorithm() const {
        return copySourceSsecAlgorithm_;
    }
    void setCopySourceSsecAlgorithm(const std::string& copySourceSsecAlgorithm) {
        copySourceSsecAlgorithm_ = copySourceSsecAlgorithm;
    }
    const std::string& getCopySourceSsecKeyMd5() const {
        return copySourceSsecKeyMd5_;
    }
    void setCopySourceSsecKeyMd5(const std::string& copySourceSsecKeyMd5) {
        copySourceSsecKeyMd5_ = copySourceSsecKeyMd5;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecAlgorithm) {
        ssecAlgorithm_ = ssecAlgorithm;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMd5_;
    }
    void setSsecKeyMd5(const std::string& ssecKeyMd5) {
        ssecKeyMd5_ = ssecKeyMd5;
    }
    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingType) {
        encodingType_ = encodingType;
    }
    const ResumableCopyCopySourceObjectInfo& getCopySourceObjectInfo() const {
        return copySourceObjectInfo_;
    }
    void setCopySourceObjectInfo(const ResumableCopyCopySourceObjectInfo& copySourceObjectInfo) {
        copySourceObjectInfo_ = copySourceObjectInfo;
    }
    const std::vector<ResumableCopyPartInfo>& getPartsInfo() const {
        return partsInfo_;
    }
    void setPartsInfo(const std::vector<ResumableCopyPartInfo>& partsInfo) {
        partsInfo_ = partsInfo;
    }
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }
    void load(const std::string& checkpointFilePath_) {
        std::ifstream ifs(checkpointFilePath_, std::ios::in);
        std::stringstream ss;
        ss << ifs.rdbuf();
        ifs.close();
        auto str = ss.str();
        if (str.empty()) {
            return;
        }
        auto j = nlohmann::json::parse(str);
        if (j.contains("Bucket"))
            j.at("Bucket").get_to(bucket_);
        if (j.contains("Key"))
            j.at("Key").get_to(key_);
        if (j.contains("PartSize"))
            j.at("PartSize").get_to(partSize_);
        if (j.contains("UploadId"))
            j.at("UploadId").get_to(uploadID_);
        if (j.contains("CopySourceIfMatch"))
            j.at("CopySourceIfMatch").get_to(copySourceIfMatch_);
        if (j.contains("CopySourceIfModifiedSince"))
            j.at("CopySourceIfModifiedSince").get_to(copySourceIfModifiedSince_);
        if (j.contains("CopySourceIfNoneMatch"))
            j.at("CopySourceIfNoneMatch").get_to(copySourceIfNoneMatch_);
        if (j.contains("CopySourceIfUnmodifiedSince"))
            j.at("CopySourceIfUnmodifiedSince").get_to(copySourceIfUnmodifiedSince_);
        if (j.contains("CopySourceSSECAlgorithm"))
            j.at("CopySourceSSECAlgorithm").get_to(copySourceSsecAlgorithm_);
        if (j.contains("CopySourceSSECKeyMD5"))
            j.at("CopySourceSSECKeyMD5").get_to(copySourceSsecKeyMd5_);
        if (j.contains("SSECAlgorithm"))
            j.at("SSECAlgorithm").get_to(ssecAlgorithm_);
        if (j.contains("SSECKeyMD5"))
            j.at("SSECKeyMD5").get_to(ssecKeyMd5_);
        if (j.contains("EncodingType"))
            j.at("EncodingType").get_to(encodingType_);

        if (j.contains("CopySourceObjectInfo")) {
            copySourceObjectInfo_.load(j.at("CopySourceObjectInfo"));
        }
        if (j.contains("PartsInfo")) {
            nlohmann::json parts = j.at("PartsInfo");
            for (auto& part : parts) {
                ResumableCopyPartInfo rfp;
                rfp.load(part);
                partsInfo_.emplace_back(rfp);
            }
        }
    }
    void dump(const std::string& checkpointFilePath_) {
        nlohmann::json j;
        j["Bucket"] = bucket_;
        j["Key"] = key_;
        j["PartSize"] = partSize_;
        j["UploadId"] = uploadID_;
        j["IfMatch"] = copySourceIfMatch_;
        j["IfModifiedSince"] = copySourceIfModifiedSince_;
        j["IfNoneMatch"] = copySourceIfNoneMatch_;
        j["IfUnmodifiedSince"] = copySourceIfUnmodifiedSince_;
        j["CopySourceSSECAlgorithm"] = copySourceSsecAlgorithm_;
        j["CopySourceSSECKeyMD5"] = copySourceSsecKeyMd5_;
        j["SSECustomerAlgorithm"] = ssecAlgorithm_;
        j["SSECustomerMD5"] = ssecKeyMd5_;
        j["EncodingType"] = encodingType_;
        j["CopySourceObjectInfo"] = copySourceObjectInfo_.dump();
        nlohmann::json jParts = nlohmann::json::array();
        for (auto& part : partsInfo_) {
            jParts.emplace_back(part.dump());
        }
        j["PartsInfo"] = jParts;
        std::ofstream ofs(checkpointFilePath_, std::ios::out | std::ios::trunc);
        ofs << j.dump();
        ofs.close();
    }

    bool isValid(const ResumableCopyObjectInput& input, const HeadObjectV2Output& output) {
        auto copySourceIfModifiedSince = TimeUtils::transTimeToGmtTime(input.getCopySourceIfModifiedSince());
        auto copySourceIfUnmodifiedSince = TimeUtils::transTimeToGmtTime(input.getCopySourceIfUnmodifiedSince());

        auto lastModified = TimeUtils::transLastModifiedStringToTime(copySourceObjectInfo_.getLastModified());
        if (input.getBucket() != bucket_ || input.getKey() != key_ || input.getPartSize() != partSize_ ||
            copySourceIfModifiedSince != copySourceIfModifiedSince_ ||
            copySourceIfUnmodifiedSince != copySourceIfUnmodifiedSince_ ||
            input.getCopySourceIfMatch() != copySourceIfMatch_ ||
            input.getCopySourceIfNoneMatch() != copySourceIfNoneMatch_ ||
            input.getCopySourceSsecAlgorithm() != copySourceSsecAlgorithm_ ||
            input.getCopySourceSsecKeyMd5() != copySourceSsecKeyMd5_ || input.getSsecAlgorithm() != ssecAlgorithm_ ||
            input.getSsecKeyMd5() != ssecKeyMd5_ || input.getEncodingType() != encodingType_ ||
            output.getETags() != copySourceObjectInfo_.getETag() ||
            output.getHashCrc64Ecma() != copySourceObjectInfo_.getHashCrc64Ecma() ||
            output.getLastModified() != lastModified) {
            return false;
        }
        return true;
    }

    void setResumableCopyPartInfoByIdx(const ResumableCopyPartInfo& resumableCopyPartInfo, int idx) {
        if (idx < 0 || idx >= partsInfo_.size())
            return;
        partsInfo_[idx] = resumableCopyPartInfo;
    }

private:
    std::string bucket_;
    std::string key_;
    int64_t partSize_ = 0;
    std::string uploadID_;
    std::string copySourceIfMatch_;
    std::string copySourceIfModifiedSince_;
    std::string copySourceIfNoneMatch_;
    std::string copySourceIfUnmodifiedSince_;
    std::string copySourceSsecAlgorithm_;
    std::string copySourceSsecKeyMd5_;
    std::string ssecAlgorithm_;
    std::string ssecKeyMd5_;
    std::string encodingType_;
    ResumableCopyCopySourceObjectInfo copySourceObjectInfo_;
    std::vector<ResumableCopyPartInfo> partsInfo_;
};
}  // namespace VolcengineTos
