#pragma once

#include <string>
#include "UploadPartOutput.h"
namespace VolcengineTos {
class DownloadFilePartInfo {
public:
    int getPartNum() const {
        return partNum_;
    }
    void setPartNum(int partnum) {
        partNum_ = partnum;
    }
    int64_t getRangeStart() const {
        return rangeStart_;
    }
    void setRangeStart(int64_t rangestart) {
        rangeStart_ = rangestart;
    }
    int64_t getRangeEnd() const {
        return rangeEnd_;
    }
    void setRangeEnd(int64_t rangeend) {
        rangeEnd_ = rangeend;
    }
    uint64_t getHashCrc64Ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64Ecma(uint64_t hashcrc64ecma) {
        hashCrc64ecma_ = hashcrc64ecma;
    }
    bool isCompleted() const {
        return isCompleted_;
    }
    void setIsCompleted(bool iscompleted) {
        isCompleted_ = iscompleted;
    }

    std::string dump();
    void load(const std::string& info);

private:
    int partNum_ = 0;
    int64_t rangeStart_ = 0;
    int64_t rangeEnd_ = 0;
    uint64_t hashCrc64ecma_ = 0;
    bool isCompleted_ = false;
};
}  // namespace VolcengineTos
