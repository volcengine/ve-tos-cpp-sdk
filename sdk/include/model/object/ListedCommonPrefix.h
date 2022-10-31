#pragma once

#include <string>

namespace VolcengineTos {
class ListedCommonPrefix {
public:
    const std::string& getPrefix() const {
        return prefix_;
    }
    void setPrefix(const std::string& prefix) {
        prefix_ = prefix;
    }

    bool operator==(const ListedCommonPrefix& rhs) const {
        return prefix_ == rhs.prefix_;
    }
    bool operator!=(const ListedCommonPrefix& rhs) const {
        return !(rhs == *this);
    }
    bool operator<(const ListedCommonPrefix& rhs) const {
        return prefix_ < rhs.prefix_;
    }
    bool operator>(const ListedCommonPrefix& rhs) const {
        return rhs < *this;
    }
    bool operator<=(const ListedCommonPrefix& rhs) const {
        return !(rhs < *this);
    }
    bool operator>=(const ListedCommonPrefix& rhs) const {
        return !(*this < rhs);
    }

private:
    std::string prefix_;
};
}  // namespace VolcengineTos
