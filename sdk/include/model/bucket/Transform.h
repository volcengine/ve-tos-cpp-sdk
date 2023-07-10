#pragma once
#include <string>
#include <utility>
#include "ReplaceKeyPrefix.h"
namespace VolcengineTos {

class Transform {
public:
    Transform() = default;
    virtual ~Transform() = default;
    const std::string& getWithKeyPrefix() const {
        return withKeyPrefix_;
    }
    void setWithKeyPrefix(const std::string& withKeyPrefix) {
        withKeyPrefix_ = withKeyPrefix;
    }
    const std::string& getWithKeySuffix() const {
        return withKeySuffix_;
    }
    void setWithKeySuffix(const std::string& withKeySuffix) {
        withKeySuffix_ = withKeySuffix;
    }
    const ReplaceKeyPrefix& getReplaceKeyPrefix() const {
        return replaceKeyPrefix_;
    }
    void setReplaceKeyPrefix(const ReplaceKeyPrefix& replaceKeyPrefix) {
        replaceKeyPrefix_ = replaceKeyPrefix;
    }

private:
    std::string withKeyPrefix_;
    std::string withKeySuffix_;
    ReplaceKeyPrefix replaceKeyPrefix_;
};

}  // namespace VolcengineTos
