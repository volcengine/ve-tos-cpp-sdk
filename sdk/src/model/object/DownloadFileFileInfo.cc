#include "../src/external/json/json.hpp"
#include "model/object/DownloadFileFileInfo.h"

void VolcengineTos::DownloadFileFileInfo::load(const std::string& info) {
    auto j = nlohmann::json::parse(info);
    if (j.contains("FilePath"))
        j.at("FilePath").get_to(filePath_);
    if (j.contains("TempFilePath"))
        j.at("TempFilePath").get_to(tempFilePath_);
}
std::string VolcengineTos::DownloadFileFileInfo::dump() {
    nlohmann::json j;
    j["FilePath"] = filePath_;
    j["TempFilePath"] = tempFilePath_;
    return j.dump();
}
