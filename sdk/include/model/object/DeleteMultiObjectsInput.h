#pragma once

#include <vector>
#include "ObjectTobeDeleted.h"
namespace VolcengineTos {
class DeleteMultiObjectsInput {
public:
    std::string toJsonString();
    const std::vector<ObjectTobeDeleted>& getObjectTobeDeleteds() const {
        return objectTobeDeleteds_;
    }
    void setObjectTobeDeleteds(const std::vector<ObjectTobeDeleted>& objectTobeDeleteds) {
        objectTobeDeleteds_ = objectTobeDeleteds;
    }
    bool isQuiet() const {
        return quiet_;
    }
    void setQuiet(bool quiet) {
        quiet_ = quiet;
    }
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }

private:
    std::vector<ObjectTobeDeleted> objectTobeDeleteds_;
    bool quiet_;
    std::string bucket_;
};
}  // namespace VolcengineTos
