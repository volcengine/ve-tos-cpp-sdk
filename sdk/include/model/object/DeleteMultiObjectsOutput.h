#pragma once

#include <vector>
#include "model/RequestInfo.h"
#include "Deleted.h"
#include "DeleteError.h"
namespace VolcengineTos {
class DeleteMultiObjectsOutput {
public:
  const RequestInfo &getRequestInfo() const { return requestInfo_; }
  void setRequestInfo(const RequestInfo &requestInfo) { requestInfo_ = requestInfo; }
  const std::vector<Deleted> &getDeleteds() const { return deleteds_; }
  const std::vector<DeleteError> &getErrors() const { return errors_; }

  void fromJsonString(const std::string& output);

private:
  RequestInfo requestInfo_;
  std::vector<Deleted> deleteds_;
  std::vector<DeleteError> errors_;
};
} // namespace VolcengineTos