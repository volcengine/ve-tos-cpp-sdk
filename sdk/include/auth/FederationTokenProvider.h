#pragma once

#include "FederationToken.h"
namespace VolcengineTos {
class FederationTokenProvider {
public:
    virtual FederationToken federationToken() {
        return {};
    }
    virtual ~FederationTokenProvider() = default;
};
}  // namespace VolcengineTos
