#pragma once

#include <string>
#include "../src/external/json/json.hpp"

namespace VolcengineTos {
class ResumableCopyPartInfo {
public:
    std::string dump() {
        nlohmann::json j;
        j["PartNum"] = partNum_;
        j["CopySourceRangeStart"] = copySourceRangeStart_;
        j["CopySourceRangeEnd"] = copySourceRangeEnd_;
        j["ETag"] = eTag_;
        j["IsCompleted"] = isCompleted_;
        return j.dump();
    }
    void load(const std::string& info) {
        auto j = nlohmann::json::parse(info);
        if (j.contains("PartNum"))
            j.at("PartNum").get_to(partNum_);
        if (j.contains("CopySourceRangeStart"))
            j.at("CopySourceRangeStart").get_to(copySourceRangeStart_);
        if (j.contains("CopySourceRangeEnd"))
            j.at("CopySourceRangeEnd").get_to(copySourceRangeEnd_);
        if (j.contains("ETag"))
            j.at("ETag").get_to(eTag_);
        if (j.contains("IsCompleted"))
            j.at("IsCompleted").get_to(isCompleted_);
    }

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
