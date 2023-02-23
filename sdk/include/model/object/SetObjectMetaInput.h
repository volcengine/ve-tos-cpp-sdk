#pragma once

#include <string>
#include "Type.h"
namespace VolcengineTos {
class SetObjectMetaInput {
public:
    SetObjectMetaInput(std::string bucket, std::string key) : bucket_(std::move(bucket)), key_(std::move(key)) {
    }
    SetObjectMetaInput() = default;
    ~SetObjectMetaInput() = default;
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
    const std::string& getContentType() const {
        return contentType_;
    }
    void setContentType(const std::string& contenttype) {
        contentType_ = contenttype;
    }
    time_t getExpires() const {
        return expires_;
    }
    void setExpires(time_t expires) {
        expires_ = expires;
    }
    const std::map<std::string, std::string>& getMeta() const {
        return meta_;
    }
    void setMeta(const std::map<std::string, std::string>& meta) {
        meta_ = meta;
    }

private:
    std::string bucket_;
    std::string key_;
    std::string versionID_;

    std::string cacheControl_;
    std::string contentDisposition_;  // TOS SDK 会对 Value 包含的中文汉字进行 URL 编码
    std::string contentEncoding_;
    std::string contentLanguage_;
    std::string contentType_;
    std::time_t expires_ = 0;

    std::map<std::string, std::string> meta_;
};
}  // namespace VolcengineTos
