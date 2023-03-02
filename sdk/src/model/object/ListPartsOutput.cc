#include "model/object/ListPartsOutput.h"
#include "../src/external/json/json.hpp"

using namespace nlohmann;
void VolcengineTos::ListPartsOutput::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("Bucket"))
        j.at("Bucket").get_to(bucket_);
    if (j.contains("Key"))
        j.at("Key").get_to(key_);
    if (j.contains("UploadId"))
        j.at("UploadId").get_to(uploadID_);
    if (j.contains("MaxParts"))
        j.at("MaxParts").get_to(maxParts_);
    if (j.contains("PartNumberMarker"))
        j.at("PartNumberMarker").get_to(partNumberMarker_);
    if (j.contains("IsTruncated"))
        j.at("IsTruncated").get_to(isTruncated_);
    // todo: encodingType API中没有相关参数
    if (j.contains("NextPartNumberMarker"))
        j.at("NextPartNumberMarker").get_to(nextPartNumberMarker_);
    if (j.contains("StorageClass"))
        setStorageClass(StringtoStorageClassType[j.at("StorageClass").get<std::string>()]);

    if (j.contains("Owner")) {
        if (j.at("Owner").contains("ID")) {
            owner_.setId(j.at("Owner").at("ID").get<std::string>());
        }
        if (j.at("Owner").contains("DisplayName")) {
            owner_.setDisplayName(j.at("Owner").at("DisplayName").get<std::string>());
        }
    }
    if (j.contains("Parts")) {
        nlohmann::json parts = j.at("Parts");
        for (auto& part : parts) {
            UploadedPartV2 up;
            if (part.contains("PartNumber"))
                up.setPartNumber(part.at("PartNumber").get<int>());
            if (part.contains("ETag"))
                up.setETag(part.at("ETag").get<std::string>());
            if (part.contains("Size"))
                up.setSize(part.at("Size").get<int>());
            if (part.contains("LastModified")) {
                std::time_t lastModified =
                        TimeUtils::transLastModifiedStringToTime(part.at("LastModified").get<std::string>());
                up.setLastModified(lastModified);
            }
            parts_.emplace_back(up);
        }
    }
}
