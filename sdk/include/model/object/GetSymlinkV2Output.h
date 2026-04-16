#pragma once

#include <string>
#include <map>
#include "TosResponse.h"
#include "Type.h"
#include "model/RequestInfo.h"

namespace VolcengineTos {
class GetSymlinkV2Output {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    const std::string& getETags() const {
        return eTag_;
    }
    void setETags(const std::string& etags) {
        eTag_ = etags;
    }
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& eTag) {
        eTag_ = eTag;
    }
    time_t getLastModified() const {
        return lastModified_;
    }
    void setLastModified(time_t lastmodified) {
        lastModified_ = lastmodified;
    }
    bool isDeleteMarker() const {
        return deleteMarker_;
    }
    void setDeleteMarker(bool deletemarker) {
        deleteMarker_ = deletemarker;
    }
    const std::string& getSsecAlgorithm() const {
        return ssecAlgorithm_;
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        ssecAlgorithm_ = ssecalgorithm;
    }
    const std::string& getSsecKeyMd5() const {
        return ssecKeyMD5_;
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        ssecKeyMD5_ = sseckeymd5;
    }
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionid) {
        versionID_ = versionid;
    }
    const std::string& getSymlinkTarget() const {
        return symlinkTarget_;
    }
    void setSymlinkTarget(const std::string& symlinkTarget) {
        symlinkTarget_ = symlinkTarget;
    }
    const std::string& getSymlinkTargetKey() const {
        return symlinkTarget_;
    }
    void setSymlinkTargetKey(const std::string& symlinkTargetKey) {
        symlinkTarget_ = symlinkTargetKey;
    }
    const std::string& getSymlinkTargetBucket() const {
        return symlinkTargetBucket_;
    }
    void setSymlinkTargetBucket(const std::string& symlinkTargetBucket) {
        symlinkTargetBucket_ = symlinkTargetBucket;
    }
    uint64_t getHashCrc64Ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64Ecma(uint64_t hashcrc64ecma) {
        hashCrc64ecma_ = hashcrc64ecma;
    }
    int64_t getSymlinkTargetSize() const {
        return symlinkTargetSize_;
    }
    void setSymlinkTargetSize(int64_t symlinkTargetSize) {
        symlinkTargetSize_ = symlinkTargetSize;
    }
    StorageClassType getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(StorageClassType storageclass) {
        storageClass_ = storageclass;
    }
    const std::map<std::string, std::string>& getMeta() const {
        return meta_;
    }
    void setMeta(const std::map<std::string, std::string>& meta) {
        meta_ = meta;
    }
    int64_t getContentLength() const {
        return contentLength_;
    }
    void setContentLength(int64_t contentlength) {
        contentLength_ = contentlength;
    }
    const std::string& getContentType() const {
        return contentType_;
    }
    void setContentType(const std::string& contenttype) {
        contentType_ = contenttype;
    }
    const std::string& getCacheControl() const {
        return cacheControl_;
    }
    void setCacheControl(const std::string& cachecontrol) {
        cacheControl_ = cachecontrol;
    }
    const std::string& getContentDisposition() const {
        return contentDisposition_;
    }
    void setContentDisposition(const std::string& contentdisposition) {
        contentDisposition_ = contentdisposition;
    }
    const std::string& getContentEncoding() const {
        return contentEncoding_;
    }
    void setContentEncoding(const std::string& contentencoding) {
        contentEncoding_ = contentencoding;
    }
    const std::string& getContentLanguage() const {
        return contentLanguage_;
    }
    void setContentLanguage(const std::string& contentlanguage) {
        contentLanguage_ = contentlanguage;
    }
    time_t getExpires() const {
        return expires_;
    }
    void setExpires(time_t expires) {
        expires_ = expires;
    }
    const std::string& getContentMd5() const {
        return contentMD5_;
    }
    void setContentMd5(const std::string& contentmd5) {
        contentMD5_ = contentmd5;
    }
    void fromResponse(TosResponse& res);

    const std::string& getStringFormatStorageClass() const {
        return StorageClassTypetoString[storageClass_];
    }

private:
    RequestInfo requestInfo_;
    std::string eTag_;
    std::time_t lastModified_ = 0;
    bool deleteMarker_ = false;
    std::string ssecAlgorithm_;
    std::string ssecKeyMD5_;
    std::string versionID_;
    std::string symlinkTarget_;
    std::string symlinkTargetBucket_;
    uint64_t hashCrc64ecma_ = 0;
    int64_t symlinkTargetSize_ = 0;
    StorageClassType storageClass_ = StorageClassType::NotSet;
    std::map<std::string, std::string> meta_;

    int64_t contentLength_ = 0;
    std::string contentType_;
    std::string cacheControl_;
    std::string contentDisposition_;
    std::string contentEncoding_;
    std::string contentLanguage_;
    std::time_t expires_ = 0;
    std::string contentMD5_;
};
}  // namespace VolcengineTos
