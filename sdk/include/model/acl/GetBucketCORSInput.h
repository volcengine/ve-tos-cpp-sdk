#pragma once

#include <string>
#include "../src/external/json/json.hpp"
namespace VolcengineTos {
class GetBucketCORSInput {
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
