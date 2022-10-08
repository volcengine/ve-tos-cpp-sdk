#pragma once

#include <string>
namespace VolcengineTos {
class DownloadFileObjectInfo {
public:
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& etag) {
        eTag_ = etag;
    }
    uint64_t getHashCrc64Ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64Ecma(uint64_t hashcrc64ecma) {
        hashCrc64ecma_ = hashcrc64ecma;
    }
    const std::string& getLastModified() const {
        return lastModified_;
    }
    void setLastModified(const std::string& lastmodified) {
        lastModified_ = lastmodified;
    }
    int64_t getObjectSize() const {
        return objectSize_;
    }
    void setObjectSize(int64_t objectsize) {
        objectSize_ = objectsize;
    }

    std::string dump() {
        nlohmann::json j;
        j["ETag"] = eTag_;
        j["HashCrc64ecma"] = hashCrc64ecma_;
        j["LastModified"] = lastModified_;
        j["ObjectSize"] = objectSize_;
        return j.dump();
    }
    void load(const std::string& info) {
        auto j = nlohmann::json::parse(info);
        if (j.contains("ETag"))
            j.at("ETag").get_to(eTag_);
        if (j.contains("HashCrc64ecma"))
            j.at("HashCrc64ecma").get_to(hashCrc64ecma_);
        if (j.contains("LastModified"))
            j.at("LastModified").get_to(lastModified_);
        if (j.contains("ObjectSize"))
            j.at("ObjectSize").get_to(objectSize_);
    }

private:
    std::string eTag_;
    uint64_t hashCrc64ecma_ = 0;
    std::string lastModified_;
    int64_t objectSize_ = 0;
};
}  // namespace VolcengineTos
