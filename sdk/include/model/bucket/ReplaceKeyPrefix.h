#pragma once
#include <string>
#include <utility>
namespace VolcengineTos {

class ReplaceKeyPrefix {
public:
    ReplaceKeyPrefix(std::string keyPrefix, std::string replaceWith)
            : keyPrefix_(std::move(keyPrefix)), replaceWith_(std::move(replaceWith)) {
    }
    ReplaceKeyPrefix() = default;
    virtual ~ReplaceKeyPrefix() = default;
    const std::string& getKeyPrefix() const {
        return keyPrefix_;
    }
    void setKeyPrefix(const std::string& keyPrefix) {
        keyPrefix_ = keyPrefix;
    }
    const std::string& getReplaceWith() const {
        return replaceWith_;
    }
    void setReplaceWith(const std::string& replaceWith) {
        replaceWith_ = replaceWith;
    }

private:
    std::string keyPrefix_;
    std::string replaceWith_;
};

}  // namespace VolcengineTos
