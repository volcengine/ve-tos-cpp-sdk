#include "../src/external/json/json.hpp"
#include "model/object/GetFileStatusOutput.h"
#include "utils/BaseUtils.h"

void VolcengineTos::GetFileStatusOutput::fromJsonString(const std::string& input) {
    auto data = nlohmann::json::parse(input);
    if (data.contains("Key")) {
        key_ = data["Key"].get<std::string>();
    }
    if (data.contains("Size")) {
        size_ = data["Size"].get<int64_t>();
    }
    if (data.contains("LastModified")) {
        lastModified_ = TimeUtils::transLastModifiedStringToTime(data["LastModified"].get<std::string>());
    }
    if (data.contains("CRC32")) {
        crc32_ = data["CRC32"].get<std::string>();
    }
    if (data.contains("CRC64")) {
        crc64_ = data["CRC64"].get<std::string>();
    }
}