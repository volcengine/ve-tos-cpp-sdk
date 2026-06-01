#pragma once

#include <memory>

#include "../../../../include/transport/TransportConfig.h"

namespace VolcengineTos {

class AsyncHttpClient;

std::shared_ptr<AsyncHttpClient> AcquireAsyncHttpClient(const TransportConfig& config, bool enable_crc);

}  // namespace VolcengineTos
