#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class GetBucketPolicyOutput {
public:
  const RequestInfo &getRequestInfo() const { return requestInfo_; }
  void setRequestInfo(const RequestInfo &requestInfo) {
    requestInfo_ = requestInfo;
  }
  const std::string &getPolicy() const { return policy_; }
  void setPolicy(const std::string &policy) { policy_ = policy; }

private:
  RequestInfo requestInfo_;
  std::string policy_;
};
} // namespace VolcengineTos