#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class AppendObjectV2Output {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getVersionID() const {
        return versionID_;
    }
    void setVersionID(const std::string& versionID) {
        versionID_ = versionID;
    }
    int64_t getNextAppendOffset() const {
        return nextAppendOffset_;
    }
    void setNextAppendOffset(int64_t nextAppendOffset) {
        nextAppendOffset_ = nextAppendOffset;
    }
    uint64_t getHashCrc64ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64ecma(uint64_t hashCrc64ecma) {
        hashCrc64ecma_ = hashCrc64ecma;
    }

private:
    RequestInfo requestInfo_;
    std::string versionID_;
    int64_t nextAppendOffset_ = 0;
    uint64_t hashCrc64ecma_ = 0;
};
}  // namespace VolcengineTos
