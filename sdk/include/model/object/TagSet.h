#pragma once

#include <string>
#include <utility>
#include <vector>

#include "model/bucket/Tag.h"

namespace VolcengineTos {
class TagSet {
public:
    TagSet() = default;
    explicit TagSet(std::vector<Tag> tags) : tags_(std::move(tags)) {
    }
    virtual ~TagSet() = default;
    const std::vector<Tag>& getTags() const {
        return tags_;
    }
    void setTags(const std::vector<Tag>& tags) {
        tags_ = tags;
    }

private:
    std::vector<Tag> tags_;
};
}  // namespace VolcengineTos
