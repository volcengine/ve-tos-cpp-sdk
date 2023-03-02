#include "model/object/ResumableCopyPartInfo.h"
#include "../src/external/json/json.hpp"

std::string VolcengineTos::ResumableCopyPartInfo::dump() {
    nlohmann::json j;
    j["PartNum"] = partNum_;
    j["CopySourceRangeStart"] = copySourceRangeStart_;
    j["CopySourceRangeEnd"] = copySourceRangeEnd_;
    j["ETag"] = eTag_;
    j["IsCompleted"] = isCompleted_;
    return j.dump();
}
void VolcengineTos::ResumableCopyPartInfo::load(const std::string& info) {
    auto j = nlohmann::json::parse(info);
    if (j.contains("PartNum"))
        j.at("PartNum").get_to(partNum_);
    if (j.contains("CopySourceRangeStart"))
        j.at("CopySourceRangeStart").get_to(copySourceRangeStart_);
    if (j.contains("CopySourceRangeEnd"))
        j.at("CopySourceRangeEnd").get_to(copySourceRangeEnd_);
    if (j.contains("ETag"))
        j.at("ETag").get_to(eTag_);
    if (j.contains("IsCompleted"))
        j.at("IsCompleted").get_to(isCompleted_);
}
