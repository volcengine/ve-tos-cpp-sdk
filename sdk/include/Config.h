#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <string>
#include "transport/TransportConfig.h"
namespace VolcengineTos {
class Config {
public:
  Config() = default;
  ~Config() = default;
  const std::string &getEndpoint() const { return endpoint_; }
  void setEndpoint(const std::string &endpoint) { endpoint_ = endpoint; }
  const std::string &getRegion() const { return region_; }
  void setRegion(const std::string &region) { region_ = region; }
  const TransportConfig &getTransportConfig() const { return transportConfig_; }
  void setTransportConfig(const TransportConfig &transportConfig) { transportConfig_ = transportConfig; }

private:
  std::string endpoint_{};
  std::string region_{};
  TransportConfig transportConfig_;
};
} // namespace VolcengineTos
#ifdef __cplusplus
}
#endif