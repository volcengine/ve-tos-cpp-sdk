#pragma once

#include <string>
namespace VolcengineTos {
class UploadFileInfoV2 {
public:
    std::string dump() {
        nlohmann::json j;
        j["LastModified"] = lastModified_;
        j["FileSize"] = fileSize_;
        return j.dump();
    }
    void load(const std::string& info) {
        auto j = nlohmann::json::parse(info);
        if (j.contains("LastModified"))
            j.at("LastModified").get_to(lastModified_);
        if (j.contains("FileSize"))
            j.at("FileSize").get_to(fileSize_);
    }

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
