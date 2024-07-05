#pragma once

#include <utility>

#include "model/RequestInfo.h"
#include "PutObjectBasicInput.h"
namespace VolcengineTos {
class ModifyObjectOutput {
public:
    ModifyObjectOutput() = default;
    ~ModifyObjectOutput() = default;
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    long getNextModifyOffset() const {
        return nextModifyOffset_;
    }
    void setNextModifyOffset(long nextmodifyoffset) {
        nextModifyOffset_ = nextmodifyoffset;
    }
    const std::string& getHashCrc64Ecma() const {
        return hashCrc64ecma_;
    }
    void setHashCrc64Ecma(const std::string& hashcrc64ecma) {
        hashCrc64ecma_ = hashcrc64ecma;
    }

private:
    RequestInfo requestInfo_;
    long nextModifyOffset_;
    std::string hashCrc64ecma_;
};
}  // namespace VolcengineTos
