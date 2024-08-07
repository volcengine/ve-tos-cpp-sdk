#pragma once

#include "model/RequestInfo.h"

namespace VolcengineTos {
class GetFileStatusOutput {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    int64_t getSize() const {
        return size_;
    }
    void setSize(int64_t size) {
        size_ = size;
    }
    std::time_t getLastModified() const {
        return lastModified_;
    }
    void setLastModified(std::time_t lastModified) {
        lastModified_ = lastModified;
    }
    const std::string& getCrc32() const {
        return crc32_;
    }
    void setCrc32(const std::string& crc32) {
        crc32_ = crc32;
    }
    const std::string& getCrc64() const {
        return crc64_;
    }
    void setCrc64(const std::string& crc64) {
        crc64_ = crc64;
    }
    void fromJsonString(const std::string& input);

private:
    RequestInfo requestInfo_;
    std::string key_;
    int64_t size_;
    std::time_t lastModified_;
    std::string crc32_;
    std::string crc64_;
};
}  // namespace VolcengineTos