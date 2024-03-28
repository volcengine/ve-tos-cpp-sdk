#pragma once

#include "transport/Transport.h"
#include "transport/http/HttpClient.h"
#include "transport/TransportConfig.h"
namespace VolcengineTos {
class SimpleTransport : public Transport {
public:
    SimpleTransport() = default;
    SimpleTransport(const TransportConfig& config);
    ~SimpleTransport() override = default;
    std::shared_ptr<TosResponse> roundTrip(const std::shared_ptr<TosRequest>& request) override;

private:
    std::shared_ptr<HttpClient> client_;
};
}  // namespace VolcengineTos
