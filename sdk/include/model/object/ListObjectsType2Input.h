#pragma once

#include <string>
#include <utility>
#include "model/GenericInput.h"

namespace VolcengineTos {
class ListObjectsType2Input : public GenericInput {
public:
    explicit ListObjectsType2Input(std::string bucket) : bucket_(std::move(bucket)) {
    }
    ListObjectsType2Input(std::string bucket, std::string prefix, int maxKeys)
            : bucket_(std::move(bucket)), prefix_(std::move(prefix)), maxKeys_(maxKeys) {
    }
    ListObjectsType2Input(std::string bucket, std::string prefix, std::string startAfter, int maxKeys)
            : bucket_(std::move(bucket)),
              prefix_(std::move(prefix)),
              startAfter_(std::move(startAfter)),
              maxKeys_(maxKeys) {
    }
    ListObjectsType2Input() = default;
    virtual ~ListObjectsType2Input() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getPrefix() const {
        return prefix_;
    }
    void setPrefix(const std::string& prefix) {
        prefix_ = prefix;
    }
    const std::string& getDelimiter() const {
        return delimiter_;
    }
    void setDelimiter(const std::string& delimiter) {
        delimiter_ = delimiter;
    }
    const std::string& getStartAfter() const {
        return startAfter_;
    }
    void setStartAfter(const std::string& startAfter) {
        startAfter_ = startAfter;
    }
    const std::string& getContinuationToken() const {
        return continuationToken_;
    }
    void setContinuationToken(const std::string& continuationToken) {
        continuationToken_ = continuationToken;
    }
    int getMaxKeys() const {
        return maxKeys_;
    }
    void setMaxKeys(int maxKeys) {
        maxKeys_ = maxKeys;
    }
    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingType) {
        encodingType_ = encodingType;
    }
    bool isListOnlyOnce() const {
        return listOnlyOnce_;
    }
    void setListOnlyOnce(bool listOnlyOnce) {
        listOnlyOnce_ = listOnlyOnce;
    }

private:
    std::string bucket_;
    std::string prefix_;
    std::string delimiter_;
    std::string startAfter_;
    std::string continuationToken_;
    int maxKeys_ = 0;
    std::string encodingType_;
    bool listOnlyOnce_ = false;
};
}  // namespace VolcengineTos
