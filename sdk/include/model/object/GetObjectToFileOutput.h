#pragma once

#include "model/RequestInfo.h"
#include "ObjectMeta.h"
#include "GetObjectBasicOutput.h"

namespace VolcengineTos {
class GetObjectToFileOutput {
public:
    void setObjectMetaFromResponse(TosResponse& response) {
        getObjectBasicOutput_.fromResponse(response);
    }
    const GetObjectBasicOutput& getGetObjectBasicOutput() const {
        return getObjectBasicOutput_;
    }
    void setGetObjectBasicOutput(const GetObjectBasicOutput& getobjectbasicoutput) {
        getObjectBasicOutput_ = getobjectbasicoutput;
    }
    const RequestInfo& getRequestInfo() const {
        return getObjectBasicOutput_.getRequestInfo();
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        getObjectBasicOutput_.setRequestInfo(requestinfo);
    }
    const std::string& getContentRange() const {
        return getObjectBasicOutput_.getContentRange();
    }
    void setContentRange(const std::string& contentrange) {
        getObjectBasicOutput_.setContentRange(contentrange);
    }
    const std::string& getETags() const {
        return getObjectBasicOutput_.getETags();
    }
    void setETag(const std::string& eTag) {
        getObjectBasicOutput_.setETag(eTag);
    }
    time_t getLastModified() const {
        return getObjectBasicOutput_.getLastModified();
    }
    void setLastModified(time_t lastmodified) {
        getObjectBasicOutput_.setLastModified(lastmodified);
    }
    bool isDeleteMarker() const {
        return getObjectBasicOutput_.isDeleteMarker();
    }
    void setDeleteMarker(bool deletemarker) {
        getObjectBasicOutput_.setDeleteMarker(deletemarker);
    }
    const std::string& getSsecAlgorithm() const {
        return getObjectBasicOutput_.getSsecAlgorithm();
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        getObjectBasicOutput_.setSsecAlgorithm(ssecalgorithm);
    }
    const std::string& getSsecKeyMd5() const {
        return getObjectBasicOutput_.getSsecKeyMd5();
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        getObjectBasicOutput_.setSsecKeyMd5(sseckeymd5);
    }
    const std::string& getVersionId() const {
        return getObjectBasicOutput_.getVersionId();
    }
    void setVersionId(const std::string& versionid) {
        getObjectBasicOutput_.setVersionId(versionid);
    }
    const std::string& getWebsiteRedirectLocation() const {
        return getObjectBasicOutput_.getWebsiteRedirectLocation();
    }
    void setWebsiteRedirectLocation(const std::string& websiteredirectlocation) {
        getObjectBasicOutput_.setWebsiteRedirectLocation(websiteredirectlocation);
    }
    const std::string& getObjectType() const {
        return getObjectBasicOutput_.getObjectType();
    }
    void setObjectType(const std::string& objecttype) {
        getObjectBasicOutput_.setObjectType(objecttype);
    }
    uint64_t getHashCrc64Ecma() const {
        return getObjectBasicOutput_.getHashCrc64Ecma();
    }
    void setHashCrc64Ecma(uint64_t hashcrc64ecma) {
        getObjectBasicOutput_.setHashCrc64Ecma(hashcrc64ecma);
    }
    StorageClassType getStorageClass() const {
        return getObjectBasicOutput_.getStorageClass();
    }
    void setStorageClass(StorageClassType storageclass) {
        getObjectBasicOutput_.setStorageClass(storageclass);
    }
    const std::map<std::string, std::string>& getMeta() const {
        return getObjectBasicOutput_.getMeta();
    }
    void setMeta(const std::map<std::string, std::string>& meta) {
        getObjectBasicOutput_.setMeta(meta);
    }
    int64_t getContentLength() const {
        return getObjectBasicOutput_.getContentLength();
    }
    void setContentLength(int64_t contentlength) {
        getObjectBasicOutput_.setContentLength(contentlength);
    }
    const std::string& getContentType() const {
        return getObjectBasicOutput_.getContentType();
    }
    void setContentType(const std::string& contenttype) {
        getObjectBasicOutput_.setContentType(contenttype);
    }
    const std::string& getCacheControl() const {
        return getObjectBasicOutput_.getCacheControl();
    }
    void setCacheControl(const std::string& cachecontrol) {
        getObjectBasicOutput_.setCacheControl(cachecontrol);
    }
    const std::string& getContentDisposition() const {
        return getObjectBasicOutput_.getContentDisposition();
    }
    void setContentDisposition(const std::string& contentdisposition) {
        getObjectBasicOutput_.setContentDisposition(contentdisposition);
    }
    const std::string& getContentEncoding() const {
        return getObjectBasicOutput_.getContentEncoding();
    }
    void setContentEncoding(const std::string& contentencoding) {
        getObjectBasicOutput_.setContentEncoding(contentencoding);
    }
    const std::string& getContentLanguage() const {
        return getObjectBasicOutput_.getContentLanguage();
    }
    void setContentLanguage(const std::string& contentlanguage) {
        getObjectBasicOutput_.setContentLanguage(contentlanguage);
    }
    time_t getExpires() const {
        return getObjectBasicOutput_.getExpires();
    }
    void setExpires(time_t expires) {
        getObjectBasicOutput_.setExpires(expires);
    }

private:
    GetObjectBasicOutput getObjectBasicOutput_;
};
}  // namespace VolcengineTos
