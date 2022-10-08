#pragma once

#include <string>
#include <Type.h>

namespace VolcengineTos {
class GetBucketMirrorBackInput {
public:
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }

private:
    std::string bucket_;
};
}  // namespace VolcengineTos
