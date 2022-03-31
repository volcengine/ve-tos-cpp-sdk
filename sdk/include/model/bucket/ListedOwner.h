
#pragma once

#include <string>
namespace VolcengineTos {
class ListedOwner {
public:
  const std::string &getId() const { return id_; }
  void setId(const std::string &id) { id_ = id; }

private:
  std::string id_;
};
} // namespace VolcengineTos\