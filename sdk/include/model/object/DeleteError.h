#pragma once

#include <string>
namespace VolcengineTos {
class DeleteError {
public:
    const std::string& getCode() const {
        return code_;
    }
    void setCode(const std::string& code) {
        code_ = code;
    }
    const std::string& getMessage() const {
        return message_;
    }
    void setMessage(const std::string& message) {
        message_ = message;
    }
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
    std::string code_;
    std::string message_;
    std::string key_;
    std::string versionID_;
};
}  // namespace VolcengineTos
