#pragma once

#include <string>
#include <utility>
namespace VolcengineTos {
class ResumableCopyCopySourceObjectInfo {
public:
    ResumableCopyCopySourceObjectInfo(std::string eTag, uint64_t hashCrc64Ecma, std::string lastModified,
                                      int64_t objectSize)
            : eTag_(std::move(eTag)),
              hashCrc64ecma_(hashCrc64Ecma),
              lastModified_(std::move(lastModified)),
              objectSize_(objectSize) {
    }
    ResumableCopyCopySourceObjectInfo() = default;
    virtual ~ResumableCopyCopySourceObjectInfo() = default;
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

    std::string dump();
    void load(const std::string& info);

private:
    std::string eTag_;
    uint64_t hashCrc64ecma_ = 0;
    std::string lastModified_;
    int64_t objectSize_ = 0;
};
}  // namespace VolcengineTos
