#pragma once

#include <string>
#include "UploadPartOutput.h"
namespace VolcengineTos {
class UploadFilePartInfo {
public:
    std::string dump();
    void load(const std::string& info);
    const UploadPartOutput& getPart() const {
        return part_;
    }
    void setPart(const UploadPartOutput& part) {
        part_ = part;
    }
    int64_t getOffset() const {
        return offset_;
    }
    void setOffset(int64_t offset) {
        offset_ = offset;
    }
    int getPartNum() const {
        return partNum_;
    }
    void setPartNum(int partNum) {
        partNum_ = partNum;
    }
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partSize) {
        partSize_ = partSize;
    }
    bool isCompleted() const {
        return isCompleted_;
    }
    void setIsCompleted(bool isCompleted) {
        isCompleted_ = isCompleted;
    }

private:
    UploadPartOutput part_;
    int64_t offset_ = 0;
    int partNum_ = 0;
    int64_t partSize_ = 0;
    bool isCompleted_ = false;
};
}  // namespace VolcengineTos
