#pragma once

#include <string>
#include <utility>
#include "Type.h"

namespace VolcengineTos {
class Tag {
public:
    Tag(std::string key, std::string value) : key_(std::move(key)), value_(std::move(value)) {
    }
    Tag() = default;
    virtual ~Tag() = default;
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getValue() const {
        return value_;
    }
    void setValue(const std::string& value) {
        value_ = value;
    }

private:
    std::string key_;
    std::string value_;
};
}  // namespace VolcengineTos
