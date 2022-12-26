#pragma once

#include <string>
#include <utility>
#include "Type.h"
#include "MirrorHeader.h"
#include "PublicSource.h"
namespace VolcengineTos {
class ErrorDocument {
public:
    explicit ErrorDocument(std::string key) : key_(std::move(key)) {
    }
    ErrorDocument() = default;
    virtual ~ErrorDocument() = default;
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }

private:
    std::string key_;
};
}  // namespace VolcengineTos
