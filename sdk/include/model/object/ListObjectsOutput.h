#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "ListedCommonPrefix.h"
#include "ListedObject.h"
namespace VolcengineTos {
class [[gnu::deprecated]] ListObjectsOutput {
public:
    void fromJsonString(const std::string& input);
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
    const std::string& getMarker() const {
        return marker_;
    }
    void setMarker(const std::string& marker) {
        marker_ = marker;
    }
    int64_t getMaxKeys() const {
        return maxKeys_;
    }
    void setMaxKeys(int64_t maxKeys) {
        maxKeys_ = maxKeys;
    }
    const std::string& getNextMarker() const {
        return nextMarker_;
    }
    void setNextMarker(const std::string& nextMarker) {
        nextMarker_ = nextMarker;
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
    const std::vector<ListedCommonPrefix>& getCommonPrefixes() const {
        return commonPrefixes_;
    }
    void setCommonPrefixes(const std::vector<ListedCommonPrefix>& commonPrefixes) {
        commonPrefixes_ = commonPrefixes;
    }
    const std::vector<ListedObject>& getContents() const {
        return contents_;
    }
    void setContents(const std::vector<ListedObject>& contents) {
        contents_ = contents;
    }

private:
    RequestInfo requestInfo_;
    std::string name_;
    std::string prefix_;
    std::string marker_;
    int64_t maxKeys_;
    std::string nextMarker_;
    std::string delimiter_;
    bool isTruncated_;
    std::string encodingType_;
    std::vector<ListedCommonPrefix> commonPrefixes_;
    std::vector<ListedObject> contents_;
};
}  // namespace VolcengineTos
