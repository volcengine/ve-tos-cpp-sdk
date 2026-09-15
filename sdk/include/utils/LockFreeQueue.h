#pragma once
#include "ConcurrentQueue.h"

namespace VolcengineTos {
// Source compatibility only: the old queue freed nodes still visible to other
// producers. This alias no longer promises lock freedom.
template <typename T>
using LockFreeQueue = ConcurrentQueue<T>;
}  // namespace VolcengineTos
