#pragma once

#include <string>
#include "Type.h"

namespace VolcengineTos {
class NoncurrentVersionExpiration {
public:
    explicit NoncurrentVersionExpiration(int noncurrentDays) : noncurrentDays_(noncurrentDays) {
    }
    NoncurrentVersionExpiration() = default;
    virtual ~NoncurrentVersionExpiration() = default;
    int getNoncurrentDays() const {
        return noncurrentDays_;
    }
    void setNoncurrentDays(int noncurrentDays) {
        noncurrentDays_ = noncurrentDays;
    }

private:
    int noncurrentDays_ = 0;
};
}  // namespace VolcengineTos
