#pragma once

#include <string>
#include "utils/BaseUtils.h"
namespace VolcengineTos {
class UploadedPartV2 {
public:
    UploadedPartV2(int partnumber, std::string etag) : partNumber_(partnumber), eTag_(std::move(etag)) {
    }
    UploadedPartV2() = default;
    ~UploadedPartV2() = default;
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partnumber) {
        partNumber_ = partnumber;
    }
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& etag) {
        eTag_ = etag;
    }
    int64_t getSize() const {
        return size_;
    }
    void setSize(int64_t size) {
        size_ = size;
    }
    time_t getLastModified() const {
        return lastModified_;
    }
    void setLastModified(time_t lastmodified) {
        lastModified_ = lastmodified;
    }
    const std::string& getStringFormatLastModified() const {
        auto lastModified = lastModified_;
        return TimeUtils::transLastModifiedTimeToString(lastModified);
    }

private:
    int partNumber_ = 0;
    std::string eTag_;
    int64_t size_ = 0;
    std::time_t lastModified_ = 0;
};
}  // namespace VolcengineTos
