#include "model/object/UploadFilePartInfo.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

std::string VolcengineTos::UploadFilePartInfo::dump() {
    json j;
    j["Part"] = part_.dump();
    j["Offset"] = offset_;
    j["PartNum"] = partNum_;
    j["PartSize"] = partSize_;
    j["IsCompleted"] = isCompleted_;
    return j.dump();
}
void VolcengineTos::UploadFilePartInfo::load(const std::string& info) {
    auto j = json::parse(info);
    UploadPartOutput output;
    if (j.contains("Part")) {
        output.load(j.at("Part"));
        setPart(output);
    }
    if (j.contains("Offset"))
        j.at("Offset").get_to(offset_);
    if (j.contains("PartNum"))
        j.at("PartNum").get_to(partNum_);
    if (j.contains("PartSize"))
        j.at("PartSize").get_to(partSize_);
    if (j.contains("IsCompleted"))
        j.at("IsCompleted").get_to(isCompleted_);
}
