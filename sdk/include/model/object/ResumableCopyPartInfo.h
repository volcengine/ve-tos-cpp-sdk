#pragma once

#include <string>

namespace VolcengineTos {
class ResumableCopyPartInfo {
public:
    std::string dump();
    void load(const std::string& info);

    int getPartNum() const {
        return partNum_;
    }
    void setPartNum(int partNum) {
        partNum_ = partNum;
    }
    int64_t getCopySourceRangeStart() const {
        return copySourceRangeStart_;
    }
    void setCopySourceRangeStart(int64_t copySourceRangeStart) {
        copySourceRangeStart_ = copySourceRangeStart;
    }
    int64_t getCopySourceRangeEnd() const {
        return copySourceRangeEnd_;
    }
    void setCopySourceRangeEnd(int64_t copySourceRangeEnd) {
        copySourceRangeEnd_ = copySourceRangeEnd;
    }
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& eTag) {
        eTag_ = eTag;
    }
    bool isCompleted() const {
        return isCompleted_;
    }
    void setIsCompleted(bool isCompleted) {
        isCompleted_ = isCompleted;
    }

private:
    int partNum_ = 0;
    int64_t copySourceRangeStart_ = 0;
    int64_t copySourceRangeEnd_ = 0;
    std::string eTag_;
    bool isCompleted_ = false;
};
}  // namespace VolcengineTos
