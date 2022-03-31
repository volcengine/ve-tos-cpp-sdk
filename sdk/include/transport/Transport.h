#pragma once

#include "TosResponse.h"
#include "TosRequest.h"
#include "TosError.h"
#include "Outcome.h"
#include "TransportConfig.h"

namespace VolcengineTos {
class Transport {
public:
  Transport() = default;
  explicit Transport(const TransportConfig &config);
  virtual ~Transport() = default;
  virtual std::shared_ptr<TosResponse> roundTrip(const std::shared_ptr<TosRequest>& request);
};
} // namespace VolcengineTos
