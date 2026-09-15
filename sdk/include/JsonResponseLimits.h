#pragma once

#include <cstddef>

namespace VolcengineTos {
// Local async parsing policy, never serialized or signed. Zero body bytes
// preserves the legacy SDK behavior; bounded callers must supply all limits.
// Bounds wire payload and JSON shape, not allocator overhead or process RSS.
struct JsonResponseLimits {
    std::size_t max_body_bytes{0};
    std::size_t max_depth{64};
    std::size_t max_events{65536};  // Keys, scalar values and container starts.
};
}  // namespace VolcengineTos
