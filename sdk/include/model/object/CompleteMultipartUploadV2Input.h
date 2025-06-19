#pragma once

#include <string>
#include <utility>
#include <vector>
#include <ostream>
#include <algorithm>
#include "UploadedPartV2.h"
#include "model/GenericInput.h"

namespace VolcengineTos {

class CompleteMultipartUploadV2Input : public GenericInput {
public:
    CompleteMultipartUploadV2Input(std::string bucket, std::string key, std::string uploadid,
                                   std::vector<UploadedPartV2> parts)
            : bucket_(std::move(bucket)),
              key_(std::move(key)),
              uploadID_(std::move(uploadid)),
              parts_(std::move(parts)) {
    }
    CompleteMultipartUploadV2Input() = default;
    ~CompleteMultipartUploadV2Input() = default;

    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }
    const std::vector<UploadedPartV2>& getParts() const {
        return parts_;
    }
    void setParts(const std::vector<UploadedPartV2>& parts) {
        parts_ = parts;
    }
    bool isCompleteAll() const {
        return completeAll_;
    }
    void setCompleteAll(bool completeAll) {
        completeAll_ = completeAll;
    }
    const std::string& getCallBack() const {
        return callBack_;
    }
    void setCallBack(const std::string& callBack) {
        callBack_ = callBack;
    }
    const std::string& getCallBackVar() const {
        return callBackVar_;
    }
    void setCallBackVar(const std::string& callBackVar) {
        callBackVar_ = callBackVar;
    }
    std::string toJsonString() const;

private:
    std::string bucket_;
    std::string key_;
    std::string uploadID_;
    std::vector<UploadedPartV2> parts_;
    bool completeAll_ = false;
    std::string callBack_;
    std::string callBackVar_;
};

}  // namespace VolcengineTos
