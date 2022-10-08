#include "transport/Transport.h"
#include "transport/TransportConfig.h"

VolcengineTos::Transport::Transport(const VolcengineTos::TransportConfig& config) {
}

std::shared_ptr<VolcengineTos::TosResponse> VolcengineTos::Transport::roundTrip(
        const std::shared_ptr<TosRequest>& request) {
    return {};
}
