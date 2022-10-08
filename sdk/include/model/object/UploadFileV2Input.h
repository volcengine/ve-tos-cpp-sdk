#pragma once
#include "CreateMultipartUploadInput.h"
#include "Type.h"
#include <string>
namespace VolcengineTos {
class UploadFileV2Input {
public:
    const CreateMultipartUploadInput& getCreateMultipartUploadInput() const {
        return createMultipartUploadInput_;
    }
    void setCreateMultipartUploadInput(const CreateMultipartUploadInput& createmultipartuploadinput) {
        createMultipartUploadInput_ = createmultipartuploadinput;
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
    const UploadEventListener& getUploadEventListener() const {
        return uploadEventListener_;
    }
    void setUploadEventListener(const UploadEventListener& uploadeventlistener) {
        uploadEventListener_ = uploadeventlistener;
    }
    const std::shared_ptr<RateLimiter>& getRateLimiter() const {
        return rateLimiter_;
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& ratelimiter) {
        rateLimiter_ = ratelimiter;
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

private:
    CreateMultipartUploadInput createMultipartUploadInput_;
    std::string filePath_;
    int64_t partSize_ = 20 * 1024 * 1024;  // 默认20MB分片大小
    int taskNum_ = 1;
    bool enableCheckpoint_ = false;
    std::string checkpointFile_;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};
    UploadEventListener uploadEventListener_ = {nullptr};
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;
    std::shared_ptr<CancelHook> cancelHook_ = nullptr;
};
}  // namespace VolcengineTos
