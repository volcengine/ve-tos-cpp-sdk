#pragma once

#include <string>
namespace VolcengineTos {
class UploadFileInfoV2 {
public:
    std::string dump();
    void load(const std::string& info);
    int64_t getLastModified() const {
        return lastModified_;
    }
    void setLastModified(int64_t lastModified) {
        lastModified_ = lastModified;
    }
    int64_t getFileSize() const {
        return fileSize_;
    }
    void setFileSize(int64_t fileSize) {
        fileSize_ = fileSize;
    }

private:
    int64_t lastModified_ = 0;
    int64_t fileSize_ = 0;
};
}  // namespace VolcengineTos
