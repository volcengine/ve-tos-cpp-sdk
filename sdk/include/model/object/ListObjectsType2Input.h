#pragma once

#include <string>
namespace VolcengineTos {
class ListObjectsType2Input {
public:
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
    bool isReverse() const {
        return reverse_;
    }
    void setReverse(bool reverse) {
        reverse_ = reverse;
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

private:
    std::string bucket_;
    std::string prefix_;
    std::string delimiter_;
    std::string startAfter_;
    std::string continuationToken_;
    bool reverse_ = false;
    int maxKeys_ = 0;
    std::string encodingType_;
};
}  // namespace VolcengineTos
