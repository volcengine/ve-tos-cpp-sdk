#include "../src/external/json/json.hpp"
#include "model/object/UploadFileInfo.h"

using namespace nlohmann;

std::string VolcengineTos::UploadFileInfo::dump() {
    json j;
    j["FilePath"] = filePath_;
    j["LastModified"] = lastModified_;
    j["FileSize"] = fileSize_;
    return j.dump();
}
void VolcengineTos::UploadFileInfo::load(const std::string& info) {
    auto j = json::parse(info);
    if (j.contains("FilePath"))
        j.at("FilePath").get_to(filePath_);
    if (j.contains("LastModified"))
        j.at("LastModified").get_to(lastModified_);
    if (j.contains("FileSize"))
        j.at("FileSize").get_to(fileSize_);
}
