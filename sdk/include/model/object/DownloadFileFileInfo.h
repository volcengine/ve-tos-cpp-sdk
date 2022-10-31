#pragma once

#include <string>
namespace VolcengineTos {
class DownloadFileFileInfo {
public:
    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }
    const std::string& getTempFilePath() const {
        return tempFilePath_;
    }
    void setTempFilePath(const std::string& tempfilepath) {
        tempFilePath_ = tempfilepath;
    }
    bool isKeyEndWithDelimiter() const {
        return keyEndWithDelimiter;
    }
    void setKeyEndWithDelimiter(bool keyEndWithDelimiter) {
        DownloadFileFileInfo::keyEndWithDelimiter = keyEndWithDelimiter;
    }
    std::string dump() {
        nlohmann::json j;
        j["FilePath"] = filePath_;
        j["TempFilePath"] = tempFilePath_;
        return j.dump();
    }
    void load(const std::string& info) {
        auto j = nlohmann::json::parse(info);
        if (j.contains("FilePath"))
            j.at("FilePath").get_to(filePath_);
        if (j.contains("TempFilePath"))
            j.at("TempFilePath").get_to(tempFilePath_);
    }

private:
    std::string filePath_;
    std::string tempFilePath_;
    bool keyEndWithDelimiter = false;
};
}  // namespace VolcengineTos
