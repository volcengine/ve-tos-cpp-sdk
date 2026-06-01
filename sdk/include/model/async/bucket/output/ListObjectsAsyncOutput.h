#pragma once

#include "model/async/base/BaseOutput.h"
#include "model/async/base/EncodingType.h"
#include "model/async/bucket/base/ListerBase.h"
#include "model/object/ListedCommonPrefix.h"
#include "model/object/ListedObjectV2.h"

#include <cstdint>
#include <string>
#include <vector>

namespace VolcengineTos {
static ListedObjectV2 parseListedObjectV2(const json& object);

class ListObjectsAsyncOutput final : public BaseOutput, public ListerBase, public EncodingType {
public:
    std::vector<ListedCommonPrefix> get_common_prefixes() const;
    std::vector<ListedObjectV2> get_contents() const;

private:
    friend class TosAsyncClient;

public:
    ListObjectsAsyncOutput() = default;
    ~ListObjectsAsyncOutput() override = default;

    const std::string& getName() const {
        return name_;
    }
    void setName(const std::string& name) {
        name_ = name;
    }
    const std::string& getNextMarker() const {
        return nextMarker_;
    }
    void setNextMarker(const std::string& nextMarker) {
        nextMarker_ = nextMarker;
    }
    bool isTruncated() const {
        return isTruncated_;
    }
    void setIsTruncated(const bool isTruncated) {
        isTruncated_ = isTruncated;
    }

    const std::vector<ListedCommonPrefix>& getCommonPrefixes() const {
        return commonPrefixes_;
    }

    const std::vector<ListedObjectV2>& getContents() const {
        return contents_;
    }

protected:
    void input2Queries() override {
    }

    void json2Output(json& j) override {
        if (j.is_discarded()) {
            return;
        }
        ListerBase::json2Output(j);
        EncodingType::json2Output(j);

        if (j.contains("Name")) {
            name_ = j["Name"].get<std::string>();
        }
        if (j.contains("IsTruncated")) {
            isTruncated_ = j["IsTruncated"].get<bool>();
        }
        if (j.contains("NextMarker")) {
            nextMarker_ = j["NextMarker"].get<std::string>();
        }

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

private:
    std::string name_;
    std::string nextMarker_;
    bool isTruncated_ = false;

    std::vector<ListedCommonPrefix> commonPrefixes_;
    std::vector<ListedObjectV2> contents_;
};

static ListedObjectV2 parseListedObjectV2(const json& object) {
    ListedObjectV2 lo;
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
        const auto hashCrc_ = object.at("HashCrc64ecma").get<std::string>();
        if (!hashCrc_.empty()) {
            lo.setHashCrc64Ecma(std::stoull(hashCrc_));
        }
    }
    return lo;
}

}  // namespace VolcengineTos
