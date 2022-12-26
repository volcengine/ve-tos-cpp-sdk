#pragma once

#include <string>
#include <utility>
namespace VolcengineTos {
class ObjectTobeDeleted {
public:
    explicit ObjectTobeDeleted(std::string key) : key_(std::move(key)) {
    }
    ObjectTobeDeleted(std::string key, std::string versionId) : key_(std::move(key)), versionID_(std::move(versionId)) {
    }
    ObjectTobeDeleted() = default;
    virtual ~ObjectTobeDeleted() = default;
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }

private:
    std::string key_;
    std::string versionID_;
};
}  // namespace VolcengineTos
