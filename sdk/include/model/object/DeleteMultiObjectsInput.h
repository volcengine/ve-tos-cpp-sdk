#pragma once

#include <vector>
#include "ObjectTobeDeleted.h"
#include "model/GenericInput.h"

namespace VolcengineTos {
class DeleteMultiObjectsInput : public GenericInput {
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
    void addObjectTobeDeleted(const ObjectTobeDeleted& objectTobeDeleted) {
        objectTobeDeleteds_.push_back(objectTobeDeleted);
    }

    bool getRecursive() const {
        return recursive_;
    }

    void setRecursive(bool isRecursive) {
        recursive_ = isRecursive;
    }

    bool getSkipTrash() const {
        return skipTrash_;
    }
    void setSkipTrash(bool skipTrash) {
        skipTrash_ = skipTrash;
    }
    const std::string& getNotificationCustomParameters() const {
        return notificationCustomParameters_;
    }
    void setNotificationCustomParameters(const std::string& notificationCustomParameters) {
        notificationCustomParameters_ = notificationCustomParameters;
    }

private:
    std::vector<ObjectTobeDeleted> objectTobeDeleteds_;
    bool quiet_;
    std::string bucket_;
    bool recursive_ = false;
    bool skipTrash_ = false;
    std::string notificationCustomParameters_;
};
}  // namespace VolcengineTos
