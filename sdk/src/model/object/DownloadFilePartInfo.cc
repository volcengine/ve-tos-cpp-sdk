#include "../src/external/json/json.hpp"

#include "model/object/DownloadFilePartInfo.h"

std::string VolcengineTos::DownloadFilePartInfo::dump() {
    nlohmann::json j;
    j["PartNum"] = partNum_;
    j["RangeStart"] = rangeStart_;
    j["RangeEnd"] = rangeEnd_;
    j["HashCrc64ecma"] = hashCrc64ecma_;
    j["IsCompleted"] = isCompleted_;
    return j.dump();
}
void VolcengineTos::DownloadFilePartInfo::load(const std::string& info) {
    auto j = nlohmann::json::parse(info);
    if (j.contains("PartNum"))
        j.at("PartNum").get_to(partNum_);
    if (j.contains("RangeStart"))
        j.at("RangeStart").get_to(rangeStart_);
    if (j.contains("RangeEnd"))
        j.at("RangeEnd").get_to(rangeEnd_);
    if (j.contains("HashCrc64ecma"))
        j.at("HashCrc64ecma").get_to(hashCrc64ecma_);
    if (j.contains("IsCompleted"))
        j.at("IsCompleted").get_to(isCompleted_);
}
