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
namespace VolcengineTos {
class DownloadFileCheckpoint {
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
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionid) {
        versionID_ = versionid;
    }
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partsize) {
        partSize_ = partsize;
    }
    const std::string& getIfMatch() const {
        return ifMatch_;
    }
    void setIfMatch(const std::string& ifmatch) {
        ifMatch_ = ifmatch;
    }
    const std::string& getIfModifiedSince() const {
        return ifModifiedSince_;
    }
    void setIfModifiedSince(const std::string& ifmodifiedsince) {
        ifModifiedSince_ = ifmodifiedsince;
    }
    const std::string& getSseAlgorithm() const {
        return sseAlgorithm_;
    }
    void setSseAlgorithm(const std::string& ssealgorithm) {
        sseAlgorithm_ = ssealgorithm;
    }
    const std::string& getSseKeyMd5() const {
        return sseKeyMd5_;
    }
    void setSseKeyMd5(const std::string& ssekeymd5) {
        sseKeyMd5_ = ssekeymd5;
    }
    const DownloadFileObjectInfo& getObjectInfo() const {
        return objectInfo_;
    }
    void setObjectInfo(const DownloadFileObjectInfo& objectinfo) {
        objectInfo_ = objectinfo;
    }
    const DownloadFileFileInfo& getFileInfo() const {
        return fileInfo_;
    }
    void setFileInfo(const DownloadFileFileInfo& fileinfo) {
        fileInfo_ = fileinfo;
    }
    const std::vector<DownloadFilePartInfo>& getPartsInfo() const {
        return partsInfo_;
    }
    void setPartsInfo(const std::vector<DownloadFilePartInfo>& partsinfo) {
        partsInfo_ = partsinfo;
    }
    const std::string& getIfNoneMatch() const {
        return ifNoneMatch_;
    }
    void setIfNoneMatch(const std::string& ifnonematch) {
        ifNoneMatch_ = ifnonematch;
    }
    const std::string& getIfUnmodifiedSince() const {
        return ifUnmodifiedSince_;
    }
    void setIfUnmodifiedSince(const std::string& ifunmodifiedsince) {
        ifUnmodifiedSince_ = ifunmodifiedsince;
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
        if (j.contains("VersionID"))
            j.at("VersionID").get_to(versionID_);
        if (j.contains("PartSize"))
            j.at("PartSize").get_to(partSize_);
        if (j.contains("IfMatch"))
            j.at("IfMatch").get_to(ifMatch_);
        if (j.contains("IfModifiedSince"))
            j.at("IfModifiedSince").get_to(ifModifiedSince_);
        if (j.contains("IfNoneMatch"))
            j.at("IfNoneMatch").get_to(ifNoneMatch_);
        if (j.contains("IfUnmodifiedSince"))
            j.at("IfUnmodifiedSince").get_to(ifUnmodifiedSince_);
        if (j.contains("SSEAlgorithm"))
            j.at("SSEAlgorithm").get_to(sseAlgorithm_);
        if (j.contains("SSECustomerMD5"))
            j.at("SSECustomerMD5").get_to(sseKeyMd5_);
        if (j.contains("ObjectInfo")) {
            objectInfo_.load(j.at("ObjectInfo"));
        }
        if (j.contains("FileInfo")) {
            fileInfo_.load(j.at("FileInfo"));
        }
        if (j.contains("PartsInfo")) {
            nlohmann::json parts = j.at("PartsInfo");
            for (auto& part : parts) {
                DownloadFilePartInfo dfp;
                dfp.load(part);
                partsInfo_.emplace_back(dfp);
            }
        }
    }
    void dump(const std::string& checkpointFilePath_) {
        nlohmann::json j;
        j["Bucket"] = bucket_;
        j["Key"] = key_;
        j["VersionID"] = versionID_;
        j["PartSize"] = partSize_;
        j["IfMatch"] = ifMatch_;
        j["IfModifiedSince"] = ifModifiedSince_;
        j["IfNoneMatch"] = ifNoneMatch_;
        j["IfUnmodifiedSince"] = ifUnmodifiedSince_;

        j["SSECustomerAlgorithm"] = sseAlgorithm_;
        j["SSECustomerMD5"] = sseKeyMd5_;
        j["ObjectInfo"] = objectInfo_.dump();
        j["FileInfo"] = fileInfo_.dump();
        nlohmann::json jParts = nlohmann::json::array();
        for (auto& part : partsInfo_) {
            jParts.emplace_back(part.dump());
        }
        j["PartsInfo"] = jParts;
        std::ofstream ofs(checkpointFilePath_, std::ios::out | std::ios::trunc);
        ofs << j.dump();
        ofs.close();
    }

    bool isValid(const HeadObjectV2Input& input, const HeadObjectV2Output& output) {
        auto headIfModifiedSince_ = TimeUtils::transTimeToGmtTime(input.getIfModifiedSince());
        auto headIfUnmodifiedSince_ = TimeUtils::transTimeToGmtTime(input.getIfUnmodifiedSince());
        auto checkPointLastModified = TimeUtils::transLastModifiedStringToTime(objectInfo_.getLastModified());
        if (input.getBucket() != bucket_ || input.getKey() != key_ || input.getIfMatch() != ifMatch_ ||
            headIfModifiedSince_ != ifModifiedSince_ || input.getIfNoneMatch() != ifNoneMatch_ ||
            headIfUnmodifiedSince_ != ifUnmodifiedSince_ || input.getVersionId() != versionID_ ||
            input.getSsecAlgorithm() != sseAlgorithm_ || input.getSsecKeyMd5() != sseKeyMd5_ ||
            output.getETags() != objectInfo_.getETag() || output.getHashCrc64Ecma() != objectInfo_.getHashCrc64Ecma() ||
            output.getLastModified() != checkPointLastModified ||
            objectInfo_.getObjectSize() != output.getContentLength()) {
            return false;
        }
        return true;
    }

    void setDownloadFilePartInfoByIdx(const DownloadFilePartInfo& downloadFilePartInfo, int idx) {
        if (idx < 0 || idx >= partsInfo_.size())
            return;
        partsInfo_[idx] = downloadFilePartInfo;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string versionID_;
    int64_t partSize_ = 0;
    std::string ifMatch_;
    std::string ifModifiedSince_;
    std::string ifNoneMatch_;
    std::string ifUnmodifiedSince_;
    std::string sseAlgorithm_;
    std::string sseKeyMd5_;
    DownloadFileObjectInfo objectInfo_;
    DownloadFileFileInfo fileInfo_;
    std::vector<DownloadFilePartInfo> partsInfo_;
};
}  // namespace VolcengineTos
