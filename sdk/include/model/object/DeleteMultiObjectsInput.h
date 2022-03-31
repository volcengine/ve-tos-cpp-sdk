#pragma once

#include <vector>
#include "ObjectTobeDeleted.h"
namespace VolcengineTos {
class DeleteMultiObjectsInput {
public:
  std::string toJsonString();
  const std::vector<ObjectTobeDeleted> &getObjectTobeDeleteds() const { return objectTobeDeleteds_; }
  void setObjectTobeDeleteds(const std::vector<ObjectTobeDeleted> &objectTobeDeleteds) {
    objectTobeDeleteds_ = objectTobeDeleteds;
  }
  bool isQuiet() const { return quiet_; }
  void setQuiet(bool quiet) { DeleteMultiObjectsInput::quiet_ = quiet; }

private:
  std::vector<ObjectTobeDeleted> objectTobeDeleteds_;
  bool quiet_;
};
} // namespace VolcengineTos