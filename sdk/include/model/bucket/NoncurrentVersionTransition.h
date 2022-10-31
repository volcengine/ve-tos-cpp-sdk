#pragma once

#include <string>
#include "Type.h"

namespace VolcengineTos {
class NoncurrentVersionTransition {
public:
    NoncurrentVersionTransition(int noncurrentDays, StorageClassType storageClass)
            : noncurrentDays_(noncurrentDays), storageClass_(storageClass) {
    }
    NoncurrentVersionTransition() = default;
    virtual ~NoncurrentVersionTransition() = default;
    int getNoncurrentDays() const {
        return noncurrentDays_;
    }
    void setNoncurrentDays(int noncurrentDays) {
        noncurrentDays_ = noncurrentDays;
    }
    StorageClassType getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(StorageClassType storageClass) {
        storageClass_ = storageClass;
    }

private:
    int noncurrentDays_ = 0;
    StorageClassType storageClass_ = StorageClassType::NotSet;
};
}  // namespace VolcengineTos
