#pragma once

#include <string>
#include "model/GenericInput.h"

namespace VolcengineTos {
class [[gnu::deprecated]] CreateBucketInput : public GenericInput {
public:
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getAcl() const {
        return acl_;
    }
    void setAcl(const std::string& acl) {
        acl_ = acl;
    }
    const std::string& getGrantFullControl() const {
        return grantFullControl_;
    }
    void setGrantFullControl(const std::string& grantFullControl) {
        grantFullControl_ = grantFullControl;
    }
    const std::string& getGrantRead() const {
        return grantRead_;
    }
    void setGrantRead(const std::string& grantRead) {
        grantRead_ = grantRead;
    }
    const std::string& getGrantReadAcp() const {
        return grantReadAcp_;
    }
    void setGrantReadAcp(const std::string& grantReadAcp) {
        grantReadAcp_ = grantReadAcp;
    }
    const std::string& getGrantWrite() const {
        return grantWrite_;
    }
    void setGrantWrite(const std::string& grantWrite) {
        grantWrite_ = grantWrite;
    }
    const std::string& getGrantWriteAcp() const {
        return grantWriteAcp_;
    }
    void setGrantWriteAcp(const std::string& grantWriteAcp) {
        grantWriteAcp_ = grantWriteAcp;
    }
    const std::string& getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(const std::string& storageClass) {
        storageClass_ = storageClass;
    }

private:
    std::string bucket_;
    std::string acl_;
    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWrite_;
    std::string grantWriteAcp_;
    std::string storageClass_;
};
}  // namespace VolcengineTos
