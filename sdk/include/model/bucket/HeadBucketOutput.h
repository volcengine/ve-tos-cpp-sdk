#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class HeadBucketOutput {
public:
  const RequestInfo &getRequestInfo() const { return requestInfo_; }
  void setRequestInfo(const RequestInfo &requestInfo) { requestInfo_ = requestInfo; }
  const std::string &getRegion() const { return region_; }
  void setRegion(const std::string &region) { region_ = region; }

private:
  RequestInfo requestInfo_;
  std::string region_;
};
} // namespace VolcengineTos