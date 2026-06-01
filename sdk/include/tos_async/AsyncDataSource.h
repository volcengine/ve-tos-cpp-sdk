#pragma once

#include <cstddef>
#include <functional>
#include <memory>

#include "AsyncEvent.h"

namespace VolcengineTos {

using OnDataReceiveWithEvent = std::function<size_t(char* data, size_t len, AsyncEvent* ev)>;
using OnDataSendWithEvent = std::function<size_t(char* data, size_t len, AsyncEvent* ev)>;

class AsyncDataSource {
public:
    virtual ~AsyncDataSource() = default;
    virtual size_t Read(char* data, size_t len, AsyncEvent* ev) = 0;
    virtual void Cancel() {}
};

using AsyncDataSourcePtr = std::shared_ptr<AsyncDataSource>;

}  // namespace VolcengineTos
