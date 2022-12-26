#pragma once

#include <string>
#include <utility>
#include "Type.h"

namespace VolcengineTos {
class FilterRule {
public:
    FilterRule(std::string name, std::string value) : name_(std::move(name)), value_(std::move(value)) {
    }
    FilterRule() = default;
    virtual ~FilterRule() = default;
    const std::string& getName() const {
        return name_;
    }
    void setName(const std::string& name) {
        name_ = name;
    }
    const std::string& getValue() const {
        return value_;
    }
    void setValue(const std::string& value) {
        value_ = value;
    }

private:
    std::string name_;
    std::string value_;
};
}  // namespace VolcengineTos
