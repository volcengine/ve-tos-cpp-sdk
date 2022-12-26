#pragma once

#include "Type.h"
#include "CreateMultipartUploadInput.h"
#include <string>
namespace VolcengineTos {
class ResumableCopyObjectInput : public CreateMultipartUploadInput {
public:
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partsize) {
        partSize_ = partsize;
    }
    int getTaskNum() const {
        return taskNum_;
    }
    void setTaskNum(int tasknum) {
        taskNum_ = tasknum;
    }
    bool isEnableCheckpoint() const {
        return enableCheckpoint_;
    }
    void setEnableCheckpoint(bool enablecheckpoint) {
        enableCheckpoint_ = enablecheckpoint;
    }
    const std::string& getCheckpointFile() const {
        return checkpointFile_;
    }
    void setCheckpointFile(const std::string& checkpointfile) {
        checkpointFile_ = checkpointfile;
    }

    const CopyEventListener& getCopyEventListener() const {
        return copyEventListener_;
    }
    void setCopyEventListener(const CopyEventListener& copyEventListener) {
        copyEventListener_ = copyEventListener;
    }

    void setCancelHook1(const std::shared_ptr<CancelHook>& cancelhook) {
        cancelHook_ = cancelhook;
    }
    const std::shared_ptr<CancelHook>& getCancelHook() const {
        return cancelHook_;
    }
    void setCancelHook(const std::shared_ptr<CancelHook>& cancelhook) {
        cancelHook_ = cancelhook;
    }
    const std::string& getSrcBucket() const {
        return srcBucket_;
    }
    void setSrcBucket(const std::string& srcBucket) {
        srcBucket_ = srcBucket;
    }
    const std::string& getSrcKey() const {
        return srcKey_;
    }
    void setSrcKey(const std::string& srcKey) {
        srcKey_ = srcKey;
    }
    const std::string& getSrcVersionId() const {
        return srcVersionId_;
    }
    void setSrcVersionId(const std::string& srcVersionId) {
        srcVersionId_ = srcVersionId;
    }
    const std::string& getCopySourceIfMatch() const {
        return copySourceIfMatch_;
    }
    void setCopySourceIfMatch(const std::string& copySourceIfMatch) {
        copySourceIfMatch_ = copySourceIfMatch;
    }
    time_t getCopySourceIfModifiedSince() const {
        return copySourceIfModifiedSince_;
    }
    void setCopySourceIfModifiedSince(time_t copySourceIfModifiedSince) {
        copySourceIfModifiedSince_ = copySourceIfModifiedSince;
    }
    const std::string& getCopySourceIfNoneMatch() const {
        return copySourceIfNoneMatch_;
    }
    void setCopySourceIfNoneMatch(const std::string& copySourceIfNoneMatch) {
        copySourceIfNoneMatch_ = copySourceIfNoneMatch;
    }
    time_t getCopySourceIfUnmodifiedSince() const {
        return copySourceIfUnmodifiedSince_;
    }
    void setCopySourceIfUnmodifiedSince(time_t copySourceIfUnmodifiedSince) {
        copySourceIfUnmodifiedSince_ = copySourceIfUnmodifiedSince;
    }
    const std::string& getCopySourceSsecAlgorithm() const {
        return copySourceSSECAlgorithm_;
    }
    void setCopySourceSsecAlgorithm(const std::string& copySourceSsecAlgorithm) {
        copySourceSSECAlgorithm_ = copySourceSsecAlgorithm;
    }
    const std::string& getCopySourceSsecKey() const {
        return copySourceSSECKey_;
    }
    void setCopySourceSsecKey(const std::string& copySourceSsecKey) {
        copySourceSSECKey_ = copySourceSsecKey;
    }
    const std::string& getCopySourceSsecKeyMd5() const {
        return copySourceSSECKeyMd5_;
    }
    void setCopySourceSsecKeyMd5(const std::string& copySourceSsecKeyMd5) {
        copySourceSSECKeyMd5_ = copySourceSsecKeyMd5;
    }

private:

    std::string srcBucket_;
    std::string srcKey_;
    std::string srcVersionId_;

    std::string copySourceIfMatch_;
    std::time_t copySourceIfModifiedSince_ = 0;
    std::string copySourceIfNoneMatch_;
    std::time_t copySourceIfUnmodifiedSince_ = 0;

    std::string copySourceSSECAlgorithm_;  // 可扩展，不定义为枚举，当前只支持 AES256，TOS SDK 会做强校验
    std::string copySourceSSECKey_;
    std::string copySourceSSECKeyMd5_;

    int64_t partSize_ = 20 * 1024 * 1024;  // 默认20MB分片大小
    int taskNum_ = 1;
    bool enableCheckpoint_ = false;
    std::string checkpointFile_;

    CopyEventListener copyEventListener_ = {nullptr};
    std::shared_ptr<CancelHook> cancelHook_ = nullptr;
};
}  // namespace VolcengineTos
