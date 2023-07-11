#pragma once
#include "Type.h"
namespace VolcengineTos {
class RestoreJobParameters {
public:
    explicit RestoreJobParameters(TierType tier) : tier_(tier) {
    }
    TierType getTier() const {
        return tier_;
    }
    void setTier(TierType tier) {
        tier_ = tier;
    }

private:
    TierType tier_ = TierType::NotSet;
};
}  // namespace VolcengineTos
