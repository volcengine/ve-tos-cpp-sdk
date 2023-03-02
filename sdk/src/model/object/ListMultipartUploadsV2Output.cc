#include "../src/external/json/json.hpp"
#include "model/object/ListMultipartUploadsV2Output.h"
#include "utils/BaseUtils.h"

void VolcengineTos::ListMultipartUploadsV2Output::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.contains("Bucket"))
        j.at("Bucket").get_to(bucket_);
    if (j.contains("KeyMarker"))
        j.at("KeyMarker").get_to(keyMarker_);
    if (j.contains("UploadIDMarker"))
        j.at("UploadIDMarker").get_to(uploadIDMarker_);
    if (j.contains("MaxUploads"))
        j.at("MaxUploads").get_to(maxUploads_);
    if (j.contains("Prefix"))
        j.at("Prefix").get_to(prefix_);
    if (j.contains("Delimiter"))
        j.at("Delimiter").get_to(delimiter_);
    if (j.contains("EncodingType"))
        j.at("EncodingType").get_to(encodingType_);
    if (j.contains("IsTruncated"))
        j.at("IsTruncated").get_to(isTruncated_);
    if (j.contains("NextKeyMarker"))
        j.at("NextKeyMarker").get_to(nextKeyMarker_);
    if (j.contains("NextUploadIDMarker"))
        j.at("NextUploadIDMarker").get_to(nextUploadIdMarker_);

    if (j.contains("CommonPrefixes")) {
        nlohmann::json commonPrefixes = j.at("CommonPrefixes");
        for (auto& commonPrefixe : commonPrefixes) {
            ListedCommonPrefix lc;
            if (commonPrefixe.contains("Prefix"))
                lc.setPrefix(commonPrefixe.at("Prefix").get<std::string>());
            commonPrefixes_.emplace_back(lc);
        }
    }
    if (j.contains("Uploads")) {
        nlohmann::json uploads = j.at("Uploads");
        for (auto& upload : uploads) {
            ListedUpload lu;
            if (upload.contains("Key"))
                lu.setKey(upload.at("Key").get<std::string>());
            if (upload.contains("UploadId"))
                lu.setUploadId(upload.at("UploadId").get<std::string>());
            if (upload.contains("StorageClass"))
                lu.setStorageClass(StringtoStorageClassType[upload.at("StorageClass").get<std::string>()]);
            if (upload.contains("Initiated"))
                lu.setInitiated(TimeUtils::transLastModifiedStringToTime(upload.at("Initiated").get<std::string>()));
            if (upload.contains("Owner")) {
                Owner owner;
                if (upload.at("Owner").contains("ID")) {
                    owner.setId(upload.at("Owner").at("ID").get<std::string>());
                }
                if (upload.at("Owner").contains("DisplayName")) {
                    owner.setDisplayName(upload.at("Owner").at("DisplayName").get<std::string>());
                }
                lu.setOwner(owner);
            }
            uploads_.emplace_back(lu);
        }
    }
}
