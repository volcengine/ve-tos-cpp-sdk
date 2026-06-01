#pragma once

#include "GetObjectAsyncInput.h"

#include <string>

namespace VolcengineTos {
class HeadObjectAsyncInput final : public GetObjectAsyncInput {
    friend class TosAsyncClient;

public:
    HeadObjectAsyncInput() = delete;
    ~HeadObjectAsyncInput() override = default;
    HeadObjectAsyncInput(const std::string& bucket, const std::string& key) : GetObjectAsyncInput(bucket, key) {
    }
};
}  // namespace VolcengineTos
