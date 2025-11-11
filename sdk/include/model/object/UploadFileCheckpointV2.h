#pragma once

#include <string>
#include <vector>
#include "UploadFilePartInfoV2.h"
#include "UploadFileInfoV2.h"
#include <sstream>
#include <fstream>
#include <iostream>
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

    void dump(std::string checkpointFilePath);
    void load(std::string checkpointFilePath);
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

        int64_t total_part_size = 0;
        for (int i = 0; i < partsInfo_.size(); i++) {
            total_part_size += partsInfo_[i].getPartSize();
        }

        return fileInfo_.getFileSize() == uploadFileSize && uploadFileSize == total_part_size &&
               fileInfo_.getLastModified() == uploadFileLastModifiedTime;
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
