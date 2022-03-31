#pragma once

#include "FederationToken.h"
namespace VolcengineTos {
class FederationTokenProvider {
public:
  virtual FederationToken federationToken() {
    return {};
  }
};
}// namespace VolcengineTos