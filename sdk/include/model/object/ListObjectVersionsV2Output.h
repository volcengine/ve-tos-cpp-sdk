#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "ListedObjectVersionV2.h"
#include "ListedCommonPrefix.h"
#include "ListedDeleteMarker.h"
namespace VolcengineTos {
class ListObjectVersionsV2Output {
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
    const std::string& getKeyMarker() const {
        return keyMarker_;
    }
    void setKeyMarker(const std::string& keyMarker) {
        keyMarker_ = keyMarker;
    }
    const std::string& getVersionIdMarker() const {
        return versionIDMarker_;
    }
    void setVersionIdMarker(const std::string& versionIdMarker) {
        versionIDMarker_ = versionIdMarker;
    }
    const std::string& getDelimiter() const {
        return delimiter_;
    }
    void setDelimiter(const std::string& delimiter) {
        delimiter_ = delimiter;
    }
    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingType) {
        encodingType_ = encodingType;
    }
    int64_t getMaxKeys() const {
        return maxKeys_;
    }
    void setMaxKeys(int64_t maxKeys) {
        maxKeys_ = maxKeys;
    }
    const std::string& getNextKeyMarker() const {
        return nextKeyMarker_;
    }
    void setNextKeyMarker(const std::string& nextKeyMarker) {
        nextKeyMarker_ = nextKeyMarker;
    }
    const std::string& getNextVersionIdMarker() const {
        return nextVersionIDMarker_;
    }
    void setNextVersionIdMarker(const std::string& nextVersionIdMarker) {
        nextVersionIDMarker_ = nextVersionIdMarker;
    }
    bool isTruncated() const {
        return isTruncated_;
    }
    void setIsTruncated(bool isTruncated) {
        isTruncated_ = isTruncated;
    }
    const std::vector<ListedCommonPrefix>& getCommonPrefixes() const {
        return commonPrefixes_;
    }
    void setCommonPrefixes(const std::vector<ListedCommonPrefix>& commonPrefixes) {
        commonPrefixes_ = commonPrefixes;
    }
    const std::vector<ListedObjectVersionV2>& getVersions() const {
        return versions_;
    }
    void setVersions(const std::vector<ListedObjectVersionV2>& versions) {
        versions_ = versions;
    }
    const std::vector<ListedDeleteMarker>& getDeleteMarkers() const {
        return deleteMarkers_;
    }
    void setDeleteMarkers(const std::vector<ListedDeleteMarker>& deleteMarkers) {
        deleteMarkers_ = deleteMarkers;
    }

private:
    RequestInfo requestInfo_;
    std::string name_;
    std::string prefix_;
    std::string keyMarker_;
    std::string versionIDMarker_;
    int64_t maxKeys_;
    std::string delimiter_;
    bool isTruncated_;
    std::string encodingType_;
    std::string nextKeyMarker_;
    std::string nextVersionIDMarker_;
    std::vector<ListedCommonPrefix> commonPrefixes_;
    std::vector<ListedObjectVersionV2> versions_;
    std::vector<ListedDeleteMarker> deleteMarkers_;
};
}  // namespace VolcengineTos
