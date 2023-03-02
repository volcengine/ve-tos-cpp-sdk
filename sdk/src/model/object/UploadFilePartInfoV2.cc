#include "../src/external/json/json.hpp"

#include "model/object/UploadFilePartInfoV2.h"

void VolcengineTos::UploadFilePartInfoV2::load(const std::string& info) {
    auto j = nlohmann::json::parse(info);
    if (j.contains("PartNum"))
        j.at("PartNum").get_to(partNum_);
    if (j.contains("PartSize"))
        j.at("PartSize").get_to(partSize_);
    if (j.contains("Offset"))
        j.at("Offset").get_to(offset_);
    if (j.contains("ETag"))
        j.at("ETag").get_to(eTag_);
    if (j.contains("HashCrc64ecma"))
        j.at("HashCrc64ecma").get_to(hashCrc64Result_);
    if (j.contains("IsCompleted"))
        j.at("IsCompleted").get_to(isCompleted_);
}
std::string VolcengineTos::UploadFilePartInfoV2::dump() {
    nlohmann::json j;
    j["PartNum"] = partNum_;
    j["PartSize"] = partSize_;
    j["Offset"] = offset_;
    j["ETag"] = eTag_;
    j["HashCrc64ecma"] = hashCrc64Result_;
    j["IsCompleted"] = isCompleted_;
    return j.dump();
}
