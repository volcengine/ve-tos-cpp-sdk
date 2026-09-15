#include "model/object/ListObjectsType2Output.h"
#include "../src/external/json/json.hpp"

#include <cstdint>
#include <limits>
#include <stdexcept>

using namespace nlohmann;

namespace {
template <typename Integer>
Integer parseNonnegativeInteger(const json& value) {
    uint64_t number = 0;
    if (value.is_number_unsigned()) {
        number = value.get<uint64_t>();
    } else if (value.is_number_integer()) {
        const auto signed_number = value.get<int64_t>();
        if (signed_number < 0) throw std::out_of_range("LIST integer must be nonnegative");
        number = static_cast<uint64_t>(signed_number);
    } else {
        // json.get<int>() also accepts floats and silently narrows integers.
        // Validate type and range before any conversion to the wire model.
        throw std::invalid_argument("LIST count or size must be an integer");
    }
    if (number > static_cast<uint64_t>(std::numeric_limits<Integer>::max()))
        throw std::out_of_range("LIST integer exceeds model range");
    return static_cast<Integer>(number);
}
}  // namespace

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
        lo.setSize(parseNonnegativeInteger<int64_t>(object.at("Size")));
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
    if (object.contains("StorageClass")) {
        const auto value = object.at("StorageClass").get<std::string>();
        const auto found = VolcengineTos::StringtoStorageClassType.find(value);
        if (found != VolcengineTos::StringtoStorageClassType.end()) lo.setStorageClass(found->second);
    }
    if (object.contains("HashCrc64ecma")) {
        auto hashCrc_ = object.at("HashCrc64ecma").get<std::string>();
        if (!hashCrc_.empty()) {
            lo.setHashCrc64Ecma(stoull(hashCrc_));
        }
    }

    return lo;
}

void VolcengineTos::ListObjectsType2Output::fromJsonString(const std::string& input) {
    fromJson(nlohmann::json::parse(input));
}

void VolcengineTos::ListObjectsType2Output::fromJson(const nlohmann::json& j, std::size_t max_entries) {
    ListObjectsType2Output parsed;
    parsed.requestInfo_ = requestInfo_;
    std::size_t entries = 0;
    for (const char* name : {"Contents", "CommonPrefixes"}) {
        if (!j.contains(name)) continue;
        const auto& array = j.at(name);
        if (!array.is_array()) throw std::invalid_argument("LIST entries must be arrays");
        if (array.size() > max_entries - entries) throw std::length_error("LIST entry limit exceeded");
        entries += array.size();
    }
    // A missing flag cannot mean the final page: that would silently truncate
    // an otherwise valid listing. at/get_to also reject null and non-booleans.
    j.at("IsTruncated").get_to(parsed.isTruncated_);
    if (j.contains("Name"))
        j.at("Name").get_to(parsed.name_);
    if (j.contains("Prefix"))
        j.at("Prefix").get_to(parsed.prefix_);
    if (j.contains("ContinuationToken"))
        j.at("ContinuationToken").get_to(parsed.continuationToken_);
    if (j.contains("MaxKeys"))
        parsed.maxKeys_ = parseNonnegativeInteger<int>(j.at("MaxKeys"));
    if (j.contains("Delimiter"))
        j.at("Delimiter").get_to(parsed.delimiter_);
    if (j.contains("EncodingType"))
        j.at("EncodingType").get_to(parsed.encodingType_);
    if (j.contains("KeyCount"))
        parsed.keyCount_ = parseNonnegativeInteger<int>(j.at("KeyCount"));
    if (j.contains("NextContinuationToken"))
        j.at("NextContinuationToken").get_to(parsed.nextContinuationToken_);
    if (j.contains("CommonPrefixes")) {
        const auto& commonPrefixes = j.at("CommonPrefixes");
        parsed.commonPrefixes_.reserve(commonPrefixes.size());
        for (const auto& cp : commonPrefixes) {
            if (!cp.is_object()) throw std::invalid_argument("LIST prefix must be an object");
            ListedCommonPrefix listedCommonPrefix;
            if (cp.contains("Prefix")) {
                listedCommonPrefix.setPrefix(cp.at("Prefix").get<std::string>());
            }
            parsed.commonPrefixes_.emplace_back(std::move(listedCommonPrefix));
        }
    }
    if (j.contains("Contents")) {
        const auto& contents = j.at("Contents");
        parsed.contents_.reserve(contents.size());
        for (const auto& ct : contents) {
            if (!ct.is_object()) throw std::invalid_argument("LIST item must be an object");
            parsed.contents_.push_back(parseListedObjectV2(ct));
        }
    }
    *this = std::move(parsed);
}
