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

private:
    RequestInfo requestInfo_;
    std::string name_;
    std::string prefix_;
    std::string continuationToken_;
    int keyCount_;
    int maxKeys_;
    std::string delimiter_;
    bool isTruncated_ = false;
    std::string encodingType_;
    std::string nextContinuationToken_;
    std::vector<ListedCommonPrefix> commonPrefixes_;
    std::vector<ListedObjectV2> contents_;
};
}  // namespace VolcengineTos
