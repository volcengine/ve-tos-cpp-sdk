#pragma once

#include <string>
#include <utility>
#include "Type.h"
namespace VolcengineTos {
class Destination {
public:
    Destination() = default;
    virtual ~Destination() = default;
    Destination(std::string bucket, std::string location) : bucket_(std::move(bucket)), location_(std::move(location)) {
    }
    Destination(std::string bucket, std::string location, StorageClassType storageClass,
                StorageClassInheritDirectiveType storageClassInheritDirective)
            : bucket_(std::move(bucket)),
              location_(std::move(location)),
              storageClass_(storageClass),
              storageClassInheritDirective_(storageClassInheritDirective) {
    }
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getLocation() const {
        return location_;
    }
    void setLocation(const std::string& location) {
        location_ = location;
    }
    StorageClassType getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(StorageClassType storageClass) {
        storageClass_ = storageClass;
    }
    StorageClassInheritDirectiveType getStorageClassInheritDirective() const {
        return storageClassInheritDirective_;
    }
    void setStorageClassInheritDirective(StorageClassInheritDirectiveType storageClassInheritDirective) {
        storageClassInheritDirective_ = storageClassInheritDirective;
    }

private:
    std::string bucket_;
    std::string location_;
    StorageClassType storageClass_ = StorageClassType::NotSet;
    StorageClassInheritDirectiveType storageClassInheritDirective_ = StorageClassInheritDirectiveType::NotSet;
};
}  // namespace VolcengineTos
