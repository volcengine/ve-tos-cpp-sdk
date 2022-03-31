
#pragma once
#ifndef TOS_ABORTMULTIPARTUPLOADOUTPUT_H
#define TOS_ABORTMULTIPARTUPLOADOUTPUT_H

#endif // TOS_ABORTMULTIPARTUPLOADOUTPUT_H

#include "model/RequestInfo.h"
namespace VolcengineTos {
class AbortMultipartUploadOutput {
public:
  const RequestInfo &getRequestInfo() const { return requestInfo_; }
  void setRequestInfo(const RequestInfo &requestInfo) {
    requestInfo_ = requestInfo;
  }

private:
  RequestInfo requestInfo_;
};
} // namespace VolcengineTos