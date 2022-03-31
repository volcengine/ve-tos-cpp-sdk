#pragma once

#include "model/RequestInfo.h"
namespace VolcengineTos {
class AppendObjectOutput {
public:
  const RequestInfo &getRequestInfo() const { return requestInfo_; }
  void setRequestInfo(const RequestInfo &requestInfo) {
    AppendObjectOutput::requestInfo_ = requestInfo;
  }
  const std::string &getEtag() const { return etag_; }
  void setEtag(const std::string &etag) { AppendObjectOutput::etag_ = etag; }
  int64_t getNextAppendOffset() const { return nextAppendOffset_; }
  void setNextAppendOffset(int64_t nextAppendOffset) {
    AppendObjectOutput::nextAppendOffset_ = nextAppendOffset;
  }

private:
  RequestInfo requestInfo_;
  std::string etag_;
  int64_t nextAppendOffset_;
};
} // namespace VolcengineTos