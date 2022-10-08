#pragma once

#include <string>
#include <utility>
#include "Type.h"
#include "GetObjectV2Input.h"
namespace VolcengineTos {
class GetObjectToFileInput {
public:
    GetObjectToFileInput(std::string bucket, std::string key, std::string filepath)
            : getObjectInput_(std::move(bucket), std::move(key)), filePath_(std::move(filepath)) {
    }
    GetObjectToFileInput() = default;
    ~GetObjectToFileInput() = default;
    
    const GetObjectV2Input& getGetObjectInput() const {
        return getObjectInput_;
    }
    void setGetObjectInput(const GetObjectV2Input& getobjectinput) {
        getObjectInput_ = getobjectinput;
    }
    const std::string& getFilePath() const {
        return filePath_;
    }
    void setFilePath(const std::string& filepath) {
        filePath_ = filepath;
    }

private:
    GetObjectV2Input getObjectInput_;
    std::string filePath_;
};
}  // namespace VolcengineTos
