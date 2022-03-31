#pragma once

#include <string>
#include "Acl.h"
namespace VolcengineTos {
class ObjectAclGrant{
private:
public:
  const std::string &getAcl() const { return acl_; }
  void setAcl(const std::string &acl) { acl_ = acl; }
  void setAclPrivate() { acl_ = ACL_PRIVATE; }
  void setAclPublicRead() { acl_ = ACL_PUBLIC_READ; }
  void setAclPublicReadWrite() { acl_ = ACL_PUBLIC_READ_WRITE; }
  void setAclAuthRead() { acl_ = ACL_AUTH_READ; }
  void setAclBucketOwnerRead() { acl_ = ACL_BUCKET_OWNER_READ; }
  void setAclBucketOwnerFullControl() { acl_ = ACL_BUCKET_OWNER_FULL_CONTROL; }
  void setAclLogDeliveryWrite() { acl_ = ACL_LOG_DELIVERY_WRITE; }
  const std::string &getGrantFullControl() const { return grantFullControl_; }
  void setGrantFullControl() { grantFullControl_ = PERMISSION_FULL_CONTROL; }
  const std::string &getGrantRead() const { return grantRead_; }
  void setGrantRead() { grantRead_ = PERMISSION_TYPE_READ; }
  const std::string &getGrantReadAcp() const { return grantReadAcp_; }
  void setGrantReadAcp() { grantReadAcp_ = PERMISSION_TYPE_READ_ACP; }
  const std::string &getGrantWrite() const { return grantWrite_; }
  void setGrantWrite() { grantWrite_ = PERMISSION_TYPE_WRITE; }
  const std::string &getGrantWriteAcp() const { return grantWriteAcp_; }
  void setGrantWriteAcp() { grantWriteAcp_ = PERMISSION_TYPE_WRITE_ACP; }

private:
  std::string acl_;
  std::string grantFullControl_;
  std::string grantRead_;
  std::string grantReadAcp_;
  std::string grantWrite_;
  std::string grantWriteAcp_;
};
} // namespace VolcengineTos