#pragma once
#include "HeadObjectV2Input.h"
#include <string>
namespace VolcengineTos {
class DownloadFileInput {
public:
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
