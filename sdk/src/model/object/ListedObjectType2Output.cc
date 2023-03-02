#include "model/object/ListObjectsType2Output.h"
#include "../src/external/json/json.hpp"

using namespace nlohmann;

VolcengineTos::ListedObjectV2 parseListedObjectV2(const json& object) {
    VolcengineTos::ListedObjectV2 lo;
    if (object.contains("Key")) {
        lo.setKey(object.at("Key").get<std::string>());
    }
    if (object.contains("LastModified")) {
        lo.setLastModified(
                VolcengineTos::TimeUtils::transLastModifiedStringToTime(object.at("LastModified").get<std::string>()));
    }

    if (object.contains("ETag"))
        lo.setETag(object.at("ETag").get<std::string>());
    if (object.contains("Size"))
        lo.setSize(object.at("Size").get<int64_t>());
    if (object.contains("Owner")) {
        VolcengineTos::Owner owner;
        if (object.at("Owner").contains("ID")) {
            owner.setId(object.at("Owner").at("ID").get<std::string>());
        }
        if (object.at("Owner").contains("DisplayName")) {
            owner.setDisplayName(object.at("Owner").at("DisplayName").get<std::string>());
        }
        lo.setOwner(owner);
    }
    if (object.contains("StorageClass"))
        lo.setStorageClass(VolcengineTos::StringtoStorageClassType[object.at("StorageClass").get<std::string>()]);
    if (object.contains("HashCrc64ecma")) {
        auto hashCrc_ = object.at("HashCrc64ecma").get<std::string>();
        if (!hashCrc_.empty()) {
            lo.setHashCrc64Ecma(stoull(hashCrc_));
        }
    }

    return lo;
}

void VolcengineTos::ListObjectsType2Output::fromJsonString(const std::string& input) {
    auto j = nlohmann::json::parse(input);
    if (j.is_discarded()) {
        return;
    }
    if (j.contains("Name"))
        j.at("Name").get_to(name_);
    if (j.contains("Prefix"))
        j.at("Prefix").get_to(prefix_);
    if (j.contains("ContinuationToken"))
        j.at("ContinuationToken").get_to(continuationToken_);
    if (j.contains("MaxKeys"))
        j.at("MaxKeys").get_to(maxKeys_);
    if (j.contains("Delimiter"))
        j.at("Delimiter").get_to(delimiter_);
    if (j.contains("EncodingType"))
        j.at("EncodingType").get_to(encodingType_);
    if (j.contains("KeyCount"))
        j.at("KeyCount").get_to(keyCount_);
    if (j.contains("IsTruncated"))
        j.at("IsTruncated").get_to(isTruncated_);
    if (j.contains("NextContinuationToken"))
        j.at("NextContinuationToken").get_to(nextContinuationToken_);
    if (j.contains("CommonPrefixes")) {
        auto commonPrefixes = j.at("CommonPrefixes");
        for (auto& cp : commonPrefixes) {
            ListedCommonPrefix listedCommonPrefix;
            if (cp.contains("Prefix")) {
                listedCommonPrefix.setPrefix(cp.at("Prefix").get<std::string>());
            }
            commonPrefixes_.emplace_back(listedCommonPrefix);
        }
    }
    if (j.contains("Contents")) {
        nlohmann::json contents = j.at("Contents");
        for (auto& ct : contents) {
            contents_.push_back(parseListedObjectV2(ct));
        }
    }
}
