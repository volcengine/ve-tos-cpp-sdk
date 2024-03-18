#include "../src/external/json/json.hpp"
#include "model/object/DownloadFileFileInfo.h"
#include "utils/BaseUtils.h"

void VolcengineTos::DownloadFileFileInfo::load(const std::string& info) {
    auto j = nlohmann::json::parse(info);
    if (j.contains("FilePath"))
        j.at("FilePath").get_to(filePath_);
    if (j.contains("TempFilePath"))
        j.at("TempFilePath").get_to(tempFilePath_);
}
std::string VolcengineTos::DownloadFileFileInfo::dump() {
    nlohmann::json j;
    j["FilePath"] = FileUtils::stringToUTF8(filePath_);
    j["TempFilePath"] = FileUtils::stringToUTF8(tempFilePath_);
    return j.dump();
}
