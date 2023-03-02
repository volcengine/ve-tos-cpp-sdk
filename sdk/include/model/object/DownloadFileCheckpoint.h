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
    void load(const std::string& checkpointFilePath_);
    void dump(const std::string& checkpointFilePath_);

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
