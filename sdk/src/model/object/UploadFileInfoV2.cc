#include "../src/external/json/json.hpp"

#include "model/object/UploadFileInfoV2.h"

std::string VolcengineTos::UploadFileInfoV2::dump() {
    nlohmann::json j;
    j["LastModified"] = lastModified_;
    j["FileSize"] = fileSize_;
    return j.dump();
}
void VolcengineTos::UploadFileInfoV2::load(const std::string& info) {
    auto j = nlohmann::json::parse(info);
    if (j.contains("LastModified"))
        j.at("LastModified").get_to(lastModified_);
    if (j.contains("FileSize"))
        j.at("FileSize").get_to(fileSize_);
}
