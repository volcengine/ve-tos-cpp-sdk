#pragma once
#include "HeadObjectV2Input.h"
#include <string>
#include <memory>
#include <utility>
namespace VolcengineTos {
class DownloadFileInput {
public:
    DownloadFileInput(std::string bucket, std::string key) : headObjectV2Input_(std::move(bucket), std::move(key)) {
    }
    DownloadFileInput(std::string bucket, std::string key, std::string versionId)
            : headObjectV2Input_(std::move(bucket), std::move(key), std::move(versionId)) {
    }
    DownloadFileInput() = default;
    virtual ~DownloadFileInput() = default;
    const HeadObjectV2Input& getHeadObjectV2Input() const {
        return headObjectV2Input_;
    }
    void setHeadObjectV2Input(const HeadObjectV2Input& headobjectv2input) {
        headObjectV2Input_ = headobjectv2input;
    }
    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }
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
    const DataTransferListener& getDataTransferListener() const {
        return dataTransferListener_;
    }
    void setDataTransferListener(const DataTransferListener& datatransferlistener) {
        dataTransferListener_ = datatransferlistener;
    }
    const DownloadEventListener& getDownloadEventListener() const {
        return downloadEventListener_;
    }
    void setDownloadEventListener(const DownloadEventListener& downloadeventlistener) {
        downloadEventListener_ = downloadeventlistener;
    }
    const std::shared_ptr<RateLimiter>& getRateLimiter() const {
        return rateLimiter_;
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& ratelimiter) {
        rateLimiter_ = ratelimiter;
    }
    const std::shared_ptr<CancelHook>& getCancelHook() const {
        return cancelHook_;
    }
    void setCancelHook(const std::shared_ptr<CancelHook>& cancelhook) {
        cancelHook_ = cancelhook;
    }

    const std::string& getBucket() const {
        return headObjectV2Input_.getBucket();
    }
    void setBucket(const std::string& bucket) {
        headObjectV2Input_.setBucket(bucket);
    }
    const std::string& getKey() const {
        return headObjectV2Input_.getKey();
    }
    void setKey(const std::string& key) {
        headObjectV2Input_.setKey(key);
    }
    const std::string& getVersionId() const {
        return headObjectV2Input_.getVersionId();
    }
    void setVersionId(const std::string& versionid) {
        headObjectV2Input_.setSsecKeyMd5(versionid);
    }
    const std::string& getIfMatch() const {
        return headObjectV2Input_.getIfMatch();
    }
    void setIfMatch(const std::string& ifmatch) {
        headObjectV2Input_.setIfMatch(ifmatch);
    }
    time_t getIfModifiedSince() const {
        return headObjectV2Input_.getIfModifiedSince();
    }
    void setIfModifiedSince(time_t ifmodifiedsince) {
        headObjectV2Input_.setIfModifiedSince(ifmodifiedsince);
    }
    const std::string& getIfNoneMatch() const {
        return headObjectV2Input_.getIfNoneMatch();
    }
    void setIfNoneMatch(const std::string& ifnonematch) {
        headObjectV2Input_.setIfNoneMatch(ifnonematch);
    }
    time_t getIfUnmodifiedSince() const {
        return headObjectV2Input_.getIfUnmodifiedSince();
    }
    void setIfUnmodifiedSince(time_t ifunmodifiedsince) {
        headObjectV2Input_.setIfUnmodifiedSince(ifunmodifiedsince);
    }
    const std::string& getSsecAlgorithm() const {
        return headObjectV2Input_.getSsecAlgorithm();
    }
    void setSsecAlgorithm(const std::string& ssecalgorithm) {
        headObjectV2Input_.setSsecAlgorithm(ssecalgorithm);
    }
    const std::string& getSsecKey() const {
        return headObjectV2Input_.getSsecKey();
    }
    void setSsecKey(const std::string& sseckey) {
        headObjectV2Input_.setSsecKey(sseckey);
    }
    const std::string& getSsecKeyMd5() const {
        return headObjectV2Input_.getSsecKeyMd5();
    }
    void setSsecKeyMd5(const std::string& sseckeymd5) {
        headObjectV2Input_.setSsecKeyMd5(sseckeymd5);
    }

private:
    HeadObjectV2Input headObjectV2Input_;
    std::string filePath_;
    int64_t partSize_ = 20 * 1024 * 1024;  // 默认20MB分片大小
    int taskNum_ = 1;
    bool enableCheckpoint_ = false;
    std::string checkpointFile_;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};
    DownloadEventListener downloadEventListener_ = {nullptr};
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;
    std::shared_ptr<CancelHook> cancelHook_ = nullptr;
};
}  // namespace VolcengineTos
