#pragma once
#ifndef TOS_DEFAULTTRANSPORT_H
#define TOS_DEFAULTTRANSPORT_H

#endif // TOS_DEFAULTTRANSPORT_H

#include "transport/Transport.h"
#include "transport/http/HttpClient.h"
#include "transport/TransportConfig.h"
namespace VolcengineTos {
class DefaultTransport : public Transport{
public:
  DefaultTransport() = default;
  DefaultTransport(const TransportConfig &config);
  ~DefaultTransport() override = default;
  std::shared_ptr<TosResponse> roundTrip(const std::shared_ptr<TosRequest>& request) override;
private:
  std::shared_ptr<HttpClient> client_;
};
} // namespace VolcengineTos