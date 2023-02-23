#include "model/object/ListUploadedPartsOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

void VolcengineTos::ListUploadedPartsOutput::fromJsonString(const std::string& input) {
    auto j = json::parse(input);
    if (j.contains("Bucket"))
        j.at("Bucket").get_to(bucket_);
    if (j.contains("Key"))
        j.at("Key").get_to(key_);
    if (j.contains("UploadId"))
        j.at("UploadId").get_to(uploadID_);
    if (j.contains("PartNumberMarker"))
        j.at("PartNumberMarker").get_to(partNumberMarker_);
    if (j.contains("NextPartNumberMarker"))
        j.at("NextPartNumberMarker").get_to(nextPartNumberMarker_);
    if (j.contains("MaxParts"))
        j.at("MaxParts").get_to(maxParts_);
    if (j.contains("IsTruncated"))
        j.at("IsTruncated").get_to(isTruncated_);
    if (j.contains("StorageClass"))
        j.at("StorageClass").get_to(storageClass_);
    if (j.contains("Owner")) {
        if (j.at("Owner").contains("ID")) {
            owner_.setId(j.at("Owner").at("ID").get<std::string>());
        }
        if (j.at("Owner").contains("DisplayName")) {
            owner_.setDisplayName(j.at("Owner").at("DisplayName").get<std::string>());
        }
    }
    if (j.contains("Parts")) {
        auto ups = j.at("Parts");
        for (auto& up : ups) {
            UploadedPart part;
            if (up.contains("PartNumber")) {
                part.setPartNumber(up.at("PartNumber").get<int>());
            }
            if (up.contains("LastModified")) {
                part.setLastModified(up.at("LastModified").get<std::string>());
            }
            if (up.contains("ETag")) {
                part.setEtag(up.at("ETag").get<std::string>());
            }
            if (up.contains("Size")) {
                part.setSize(up.at("Size").get<int64_t>());
            }
            uploadedParts_.emplace_back(part);
        }
    }
}