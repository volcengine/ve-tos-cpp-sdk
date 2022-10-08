#pragma once

#include <string>
namespace VolcengineTos {
class UploadFileInput {
public:
    const std::string& getObjectKey() const {
        return objectKey_;
    }
    void setObjectKey(const std::string& objectKey) {
        objectKey_ = objectKey;
    }
    const std::string& getUploadFilePath() const {
        return uploadFilePath_;
    }
    void setUploadFilePath(const std::string& uploadFilePath) {
        uploadFilePath_ = uploadFilePath;
    }
    int64_t getPartSize() const {
        return partSize_;
    }
    void setPartSize(int64_t partSize) {
        partSize_ = partSize;
    }
    int getTaskNum() const {
        return taskNum_;
    }
    void setTaskNum(int taskNum) {
        taskNum_ = taskNum;
    }
    bool isEnableCheckpoint() const {
        return enableCheckpoint_;
    }
    void setEnableCheckpoint(bool enableCheckpoint) {
        enableCheckpoint_ = enableCheckpoint;
    }
    const std::string& getCheckpointFile() const {
        return checkpointFile_;
    }
    void setCheckpointFile(const std::string& checkpointFile) {
        checkpointFile_ = checkpointFile;
    }

private:
    std::string objectKey_;
    std::string uploadFilePath_;
    // 默认20MB分片大小
    int64_t partSize_ = 20 * 1024 * 1024;
    int taskNum_ = 1;
    bool enableCheckpoint_ = false;
    std::string checkpointFile_;
};
}  // namespace VolcengineTos
