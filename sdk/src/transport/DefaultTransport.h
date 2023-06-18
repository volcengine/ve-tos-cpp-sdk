#pragma once

#include "transport/Transport.h"
#include "transport/http/HttpClient.h"
#include "transport/TransportConfig.h"
namespace VolcengineTos {
class DefaultTransport : public Transport {
public:
    DefaultTransport() = default;
    DefaultTransport(const TransportConfig& config);
    ~DefaultTransport() override = default;
    std::shared_ptr<TosResponse> roundTrip(const std::shared_ptr<TosRequest>& request) override;

private:
    std::shared_ptr<HttpClient> client_;
};
}  // namespace VolcengineTos
