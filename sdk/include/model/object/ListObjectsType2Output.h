#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "ListedCommonPrefix.h"
#include "ListedObjectV2.h"
namespace VolcengineTos {
class ListObjectsType2Output {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestInfo) {
        requestInfo_ = requestInfo;
    }
    const std::string& getName() const {
        return name_;
    }
    void setName(const std::string& name) {
        name_ = name;
    }
    const std::string& getPrefix() const {
        return prefix_;
    }
    void setPrefix(const std::string& prefix) {
        prefix_ = prefix;
    }
    const std::string& getContinuationToken() const {
        return continuationToken_;
    }
    void setContinuationToken(const std::string& continuationToken) {
        continuationToken_ = continuationToken;
    }
    int getKeyCount() const {
        return keyCount_;
    }
    void setKeyCount(int keyCount) {
        keyCount_ = keyCount;
    }
    int getMaxKeys() const {
        return maxKeys_;
    }
    void setMaxKeys(int maxKeys) {
        maxKeys_ = maxKeys;
    }
    const std::string& getDelimiter() const {
        return delimiter_;
    }
    void setDelimiter(const std::string& delimiter) {
        delimiter_ = delimiter;
    }
    bool isTruncated() const {
        return isTruncated_;
    }
    void setIsTruncated(bool isTruncated) {
        isTruncated_ = isTruncated;
    }
    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingType) {
        encodingType_ = encodingType;
    }
    const std::string& getNextContinuationToken() const {
        return nextContinuationToken_;
    }
    void setNextContinuationToken(const std::string& nextContinuationToken) {
        nextContinuationToken_ = nextContinuationToken;
    }
    const std::vector<ListedCommonPrefix>& getCommonPrefixes() const {
        return commonPrefixes_;
    }
    void setCommonPrefixes(const std::vector<ListedCommonPrefix>& commonPrefixes) {
        commonPrefixes_ = commonPrefixes;
    }
    const std::vector<ListedObjectV2>& getContents() const {
        return contents_;
    }
    void setContents(const std::vector<ListedObjectV2>& contents) {
        contents_ = contents;
    }

    void fromJsonString(const std::string& input) {
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
                contents_.push_back(ListedObjectV2::parseListedObjectV2(ct));
            }
        }
    }

private:
    RequestInfo requestInfo_;
    std::string name_;
    std::string prefix_;
    std::string continuationToken_;
    int maxKeys_ = 0;
    std::string delimiter_;
    std::string encodingType_;
    int keyCount_ = 0;
    bool isTruncated_ = false;
    std::string nextContinuationToken_;
    std::vector<ListedCommonPrefix> commonPrefixes_;
    std::vector<ListedObjectV2> contents_;
};
}  // namespace VolcengineTos
