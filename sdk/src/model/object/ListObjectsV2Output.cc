#include "model/object/ListObjectsV2Output.h"
#include "../src/external/json/json.hpp"
#include "utils/BaseUtils.h"
#include <string>
#include <iostream>
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
    if (object.contains("HashCrc64ecma"))
        lo.setHashCrc64Ecma(stoull(object.at("HashCrc64ecma").get<std::string>()));
    return lo;
}

void VolcengineTos::ListObjectsV2Output::fromJsonString(const std::string& input) {
    auto j = json::parse(input);
    if (j.contains("Name"))
        j.at("Name").get_to(name_);
    if (j.contains("Prefix"))
        j.at("Prefix").get_to(prefix_);
    if (j.contains("Marker"))
        j.at("Marker").get_to(marker_);
    if (j.contains("MaxKeys"))
        j.at("MaxKeys").get_to(maxKeys_);
    if (j.contains("Delimiter"))
        j.at("Delimiter").get_to(delimiter_);
    if (j.contains("IsTruncated"))
        j.at("IsTruncated").get_to(isTruncated_);
    if (j.contains("EncodingType"))
        j.at("EncodingType").get_to(encodingType_);
    if (j.contains("NextMarker"))
        j.at("NextMarker").get_to(nextMarker_);
    if (j.contains("CommonPrefixes")) {
        json pres = j.at("CommonPrefixes");
        for (auto& pre : pres) {
            ListedCommonPrefix lcp;
            std::string prefix;
            if (pre.contains("Prefix"))
                pre.at("Prefix").get_to(prefix);
            lcp.setPrefix(prefix);
            commonPrefixes_.emplace_back(lcp);
        }
    }

    if (j.contains("Contents")) {
        json contents = j.at("Contents");
        for (auto& ct : contents) {
            contents_.push_back(parseListedObjectV2(ct));
        }
    }
}
