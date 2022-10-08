#pragma once

#include <string>
namespace VolcengineTos {
class ObjectTobeDeleted {
public:
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
