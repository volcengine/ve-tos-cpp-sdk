#pragma once

#include <string>
#include <Type.h>

namespace VolcengineTos {
class CreateBucketV2Input {
public:
    explicit CreateBucketV2Input(std::string bucket) : bucket_(std::move(bucket)) {
    }
    CreateBucketV2Input() = default;
    ~CreateBucketV2Input() = default;
    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const ACLType& getAcl() const {
        return acl_;
    }
    void setAcl(const ACLType& acl) {
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
    const StorageClassType& getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(const StorageClassType& storageClass) {
        storageClass_ = storageClass;
    }
    const AzRedundancyType& getAzRedundancy() const {
        return azRedundancy_;
    }
    void setAzRedundancy(const AzRedundancyType& azRedundancy) {
        azRedundancy_ = azRedundancy;
    }

private:
    std::string bucket_;
    ACLType acl_ = ACLType::Private;
    std::string grantFullControl_;
    std::string grantRead_;
    std::string grantReadAcp_;
    std::string grantWrite_;
    std::string grantWriteAcp_;
    StorageClassType storageClass_ = StorageClassType::STANDARD;
    AzRedundancyType azRedundancy_ = AzRedundancyType::NotSet;
};
}  // namespace VolcengineTos
