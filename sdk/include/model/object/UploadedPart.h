#pragma once

#include <string>
namespace VolcengineTos {
class UploadedPart {
public:
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partNumber) {
        partNumber_ = partNumber;
    }
    const std::string& getLastModified() const {
        return lastModified_;
    }
    void setLastModified(const std::string& lastModified) {
        lastModified_ = lastModified;
    }
    const std::string& getEtag() const {
        return etag_;
    }
    void setEtag(const std::string& etag) {
        etag_ = etag;
    }
    int64_t getSize() const {
        return size_;
    }
    void setSize(int64_t size) {
        size_ = size;
    }

private:
    int partNumber_;
    std::string lastModified_;
    std::string etag_;
    int64_t size_;
};
}  // namespace VolcengineTos
