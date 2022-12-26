#pragma once

#include <string>
#include "Type.h"
#include "FilterKey.h"

namespace VolcengineTos {
class Filter {
public:
    explicit Filter(const FilterKey& key) : key_(key) {
    }
    Filter() = default;
    virtual ~Filter() = default;
    const FilterKey& getKey() const {
        return key_;
    }
    void setKey(const FilterKey& key) {
        key_ = key;
    }

private:
    FilterKey key_;
};
}  // namespace VolcengineTos
