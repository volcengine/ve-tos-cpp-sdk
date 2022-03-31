#pragma once

#include <string>

namespace VolcengineTos {
class ListedCommonPrefix {
public:
  const std::string &getPrefix() const { return prefix_; }
  void setPrefix(const std::string &prefix) { prefix_ = prefix; }

private:
  std::string prefix_;
};
} // namespace VolcengineTos