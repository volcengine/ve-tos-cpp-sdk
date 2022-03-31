#include "transport/TransportConfig.h"
namespace VolcengineTos {
TransportConfig::TransportConfig() {
  maxIdleCount_ = 128;
  requestTimeout_ = 10;
  dialTimeout_ = 10;
  keepAlive_ = 30;
  connectTimeout_ = 60;
  tlsHandshakeTimeout_ = 10;
  responseHeaderTimeout_ = 60;
  expectContinueTimeout_ = 3;
  readTimeout_ = 120;
  writeTimeout_ = 120;
}
} // namespace VolcengineTos
