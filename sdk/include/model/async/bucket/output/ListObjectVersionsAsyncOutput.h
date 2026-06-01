#pragma once
#include "model/async/base/BaseOutput.h"
#include "model/async/base/EncodingType.h"
#include "model/async/bucket/base/VersionsLister.h"
#include "model/object/ListedCommonPrefix.h"
#include "model/object/ListedDeleteMarker.h"
#include "model/object/ListedObjectV2.h"
#include "model/object/ListedObjectVersionV2.h"

#include <cstdint>
#include <string>
#include <vector>

namespace VolcengineTos {
static Owner parseOwnerV2(const json& o);
static ListedDeleteMarker parseListedDeleteMarkerEntryV2(const json& marker);
static ListedObjectVersionV2 parseListedObjectVersion(const json& version);
class ListObjectVersionsAsyncOutput final : public BaseOutput, public VersionsListerBase, public EncodingType {
    friend class TosAsyncClient;

public:
    ListObjectVersionsAsyncOutput() = default;
    ~ListObjectVersionsAsyncOutput() override = default;

    const std::string& getName() const {
        return name_;
    }
    void setName(const std::string& name) {
        name_ = name;
    }

    bool isTruncated() const {
        return isTruncated_;
    }
    void setIsTruncated(const bool isTruncated) {
        isTruncated_ = isTruncated;
    }

protected:
    void input2Queries() override {
    }

    void json2Output(json& j) override {
        if (j.is_discarded()) {
            return;
        }
        VersionsListerBase::json2Output(j);
        EncodingType::json2Output(j);

        if (j.contains("Name")) {
            name_ = j["Name"].get<std::string>();
        }
        if (j.contains("IsTruncated")) {
            isTruncated_ = j["IsTruncated"].get<bool>();
        }
        if (j.contains("NextKeyMarker")) {
            nextKeyMarker_ = j["NextKeyMarker"].get<std::string>();
        }
        if (j.contains("NextVersionIdMarker")) {
            nextVersionIDMarker_ = j["NextVersionIdMarker"].get<std::string>();
        }

        if (j.contains("CommonPrefixes")) {
            json pres = j.at("CommonPrefixes");
            for (auto& pre : pres) {
                ListedCommonPrefix lcp;
                if (pre.contains("Prefix")) {
                    lcp.setPrefix(pre.at("Prefix").get<std::string>());
                }
                commonPrefixes_.emplace_back(lcp);
            }
        }
        if (j.contains("Versions")) {
            json versions = j.at("Versions");
            for (auto& v : versions) {
                versions_.emplace_back(parseListedObjectVersion(v));
            }
        }
        if (j.contains("DeleteMarkers")) {
            json dms = j.at("DeleteMarkers");
            for (auto& dm : dms) {
                deleteMarkers_.emplace_back(parseListedDeleteMarkerEntryV2(dm));
            }
        }
    }

private:
    std::string name_;
    bool isTruncated_;
    int keyCount_ = 0;

    std::string nextKeyMarker_;
    std::string nextVersionIDMarker_;

    std::vector<ListedCommonPrefix> commonPrefixes_;
    std::vector<ListedObjectVersionV2> versions_;
    std::vector<ListedDeleteMarker> deleteMarkers_;
};

static Owner parseOwnerV2(const json& o) {
    Owner owner;
    if (o.contains("ID")) {
        owner.setId(o.at("ID").get<std::string>());
    }
    if (o.contains("DisplayName")) {
        owner.setDisplayName(o.at("DisplayName").get<std::string>());
    }
    return owner;
}

static ListedDeleteMarker parseListedDeleteMarkerEntryV2(const json& marker) {
    ListedDeleteMarker entry;
    if (marker.contains("Key"))
        entry.setKey(marker.at("Key").get<std::string>());
    if (marker.contains("IsLatest"))
        entry.setIsLatest(marker.at("IsLatest").get<bool>());
    if (marker.contains("LastModified")) {
        entry.setLastModified(TimeUtils::transLastModifiedStringToTime(marker.at("LastModified").get<std::string>()));
    }
    if (marker.contains("Owner")) {
        entry.setOwner(parseOwnerV2(marker.at("Owner")));
    }
    if (marker.contains("VersionId"))
        entry.setVersionId(marker.at("VersionId").get<std::string>());
    return entry;
}

static ListedObjectVersionV2 parseListedObjectVersion(const json& version) {
    ListedObjectVersionV2 listedObjectVersion;
    if (version.contains("Key"))
        listedObjectVersion.setKey(version.at("Key").get<std::string>());
    if (version.contains("LastModified"))
        listedObjectVersion.setLastModified(
                TimeUtils::transLastModifiedStringToTime(version.at("LastModified").get<std::string>()));
    if (version.contains("ETag"))
        listedObjectVersion.setVersionId(version.at("ETag").get<std::string>());
    if (version.contains("IsLatest"))
        listedObjectVersion.setIsLatest(version.at("IsLatest").get<bool>());
    if (version.contains("Size"))
        listedObjectVersion.setSize(version.at("Size").get<int64_t>());
    if (version.contains("Owner")) {
        listedObjectVersion.setOwner(parseOwnerV2(version.at("Owner")));
    }
    if (version.contains("StorageClass"))
        listedObjectVersion.setStorageClass(StringtoStorageClassType[version.at("StorageClass").get<std::string>()]);
    if (version.contains("VersionId"))
        listedObjectVersion.setVersionId(version.at("VersionId").get<std::string>());
    if (version.contains("HashCrc64ecma")) {
        const auto hashCrc_ = version.at("HashCrc64ecma").get<std::string>();
        if (!hashCrc_.empty()) {
            listedObjectVersion.setHashCrc64Ecma(std::stoull(hashCrc_));
        }
    }

    return listedObjectVersion;
}

}  // namespace VolcengineTos
