#pragma once

#include <string>
#include <Type.h>
#include <vector>
#include "MirrorBackRule.h"

namespace VolcengineTos {
class PutBucketLifecycleInput {
public:
private:
    std::string bucket_;
    std::vector<MirrorBackRule> rules_;
};
}  // namespace VolcengineTos
