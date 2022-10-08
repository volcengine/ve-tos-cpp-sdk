#pragma once

#include <string>
#include <vector>
#include "UploadFilePartInfoV2.h"
#include "UploadFileInfoV2.h"
namespace VolcengineTos {
class UploadFileCheckpointV2 {
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
    void setPartSize(int64_t partsize) {
        partSize_ = partsize;
    }
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadid) {
        uploadID_ = uploadid;
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
    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingtype) {
        encodingType_ = encodingtype;
    }
    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }
    const UploadFileInfoV2& getFileInfo() const {
        return fileInfo_;
    }
    void setFileInfo(const UploadFileInfoV2& fileinfo) {
        fileInfo_ = fileinfo;
    }
    const std::vector<UploadFilePartInfoV2>& getPartsInfo() const {
        return partsInfo_;
    }
    void setPartsInfo(const std::vector<UploadFilePartInfoV2>& partsinfo) {
        partsInfo_ = partsinfo;
    }

    void dump(std::string checkpointFilePath) {
        nlohmann::json j;
        j["Bucket"] = bucket_;
        j["Key"] = key_;
        j["PartSize"] = partSize_;
        j["UploadID"] = uploadID_;
        j["SSECustomerAlgorithm"] = sseAlgorithm_;
        j["SSECustomerMD5"] = sseKeyMd5_;
        j["EncodingType"] = encodingType_;
        j["FilePath"] = filePath_;
        j["FileInfo"] = fileInfo_.dump();
        nlohmann::json jParts = nlohmann::json::array();
        for (auto& part : partsInfo_) {
            jParts.emplace_back(part.dump());
        }
        j["PartsInfo"] = jParts;
        std::ofstream ofs(checkpointFilePath, std::ios::out | std::ios::trunc);
        ofs << j.dump();
        ofs.close();
    }

    void load(std::string checkpointFilePath) {
        if (checkpointFilePath.empty()) {
            return;
        }
        std::ifstream ifs(checkpointFilePath, std::ios::in);
        std::stringstream ss;
        ss << ifs.rdbuf();
        ifs.close();
        auto str = ss.str();
        if (str.empty()) {
            return;
        }
        auto j = nlohmann::json::parse(ss.str());
        if (j.contains("Bucket"))
            j.at("Bucket").get_to(bucket_);
        if (j.contains("Key"))
            j.at("Key").get_to(key_);
        if (j.contains("PartSize"))
            j.at("PartSize").get_to(partSize_);
        if (j.contains("UploadID"))
            j.at("UploadID").get_to(uploadID_);
        if (j.contains("SSECustomerAlgorithm"))
            j.at("SSECustomerAlgorithm").get_to(sseAlgorithm_);
        if (j.contains("SSECustomerMD5"))
            j.at("SSECustomerMD5").get_to(sseKeyMd5_);
        if (j.contains("EncodingType"))
            j.at("EncodingType").get_to(encodingType_);
        if (j.contains("FilePath"))
            j.at("FilePath").get_to(filePath_);
        if (j.contains("FileInfo")) {
            fileInfo_.load(j.at("FileInfo"));
        }
        if (j.contains("PartsInfo")) {
            nlohmann::json parts = j.at("PartsInfo");
            for (auto& part : parts) {
                UploadFilePartInfoV2 ufp;
                ufp.load(part);
                partsInfo_.emplace_back(ufp);
            }
        }
    }
    void setUploadFilePartInfoByIdx(const UploadFilePartInfoV2& uploadFilePartInfo, int idx) {
        if (idx < 0 || idx >= partsInfo_.size())
            return;
        partsInfo_[idx] = uploadFilePartInfo;
    }
    bool isValid(int64_t uploadFileSize, int64_t uploadFileLastModifiedTime, const std::string& bucket,
                 const std::string& objectKey) {
        if (uploadID_.empty() || bucket_ != bucket || key_ != objectKey) {
            return false;
        }
        return fileInfo_.getFileSize() == uploadFileSize && fileInfo_.getLastModified() == uploadFileLastModifiedTime;
    }

private:
    std::string bucket_;
    std::string key_;
    int64_t partSize_ = 0;
    std::string uploadID_;
    std::string sseAlgorithm_;
    std::string sseKeyMd5_;
    std::string encodingType_;
    std::string filePath_;
    UploadFileInfoV2 fileInfo_;
    std::vector<UploadFilePartInfoV2> partsInfo_;
};
}  // namespace VolcengineTos
