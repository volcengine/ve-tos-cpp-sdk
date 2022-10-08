#pragma once

#include <string>
#include "UploadPartOutput.h"
namespace VolcengineTos {
class UploadFilePartInfoV2 {
public:
    std::string dump() {
        nlohmann::json j;
        j["PartNum"] = partNum_;
        j["PartSize"] = partSize_;
        j["Offset"] = offset_;
        j["ETag"] = eTag_;
        j["HashCrc64ecma"] = hashCrc64Result_;
        j["IsCompleted"] = isCompleted_;
        return j.dump();
    }
    void load(const std::string& info) {
        auto j = nlohmann::json::parse(info);
        if (j.contains("PartNum"))
            j.at("PartNum").get_to(partNum_);
        if (j.contains("PartSize"))
            j.at("PartSize").get_to(partSize_);
        if (j.contains("Offset"))
            j.at("Offset").get_to(offset_);
        if (j.contains("ETag"))
            j.at("ETag").get_to(eTag_);
        if (j.contains("HashCrc64ecma"))
            j.at("HashCrc64ecma").get_to(hashCrc64Result_);
        if (j.contains("IsCompleted"))
            j.at("IsCompleted").get_to(isCompleted_);
    }

    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& etag) {
        eTag_ = etag;
    }
    uint64_t getHashCrc64Result() const {
        return hashCrc64Result_;
    }
    void setHashCrc64Result(uint64_t hashcrc64result) {
        hashCrc64Result_ = hashcrc64result;
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
    int partNum_ = 0;
    int64_t partSize_ = 0;
    int64_t offset_ = 0;
    std::string eTag_;
    uint64_t hashCrc64Result_ = 0;
    bool isCompleted_ = false;
};
}  // namespace VolcengineTos
