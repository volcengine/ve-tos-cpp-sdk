// Keep the SDK header first so standard includes below cannot hide missing dependencies.
#include "TosResponse.h"

#include <limits>
#include <type_traits>

static_assert(std::is_same<decltype(std::declval<VolcengineTos::TosResponse>().getHashCrc64Result()), uint64_t>::value,
              "CRC64 getter must retain its 64-bit unsigned type");
static_assert(std::is_same<decltype(&VolcengineTos::TosResponse::setHashCrc64Result),
                           void (VolcengineTos::TosResponse::*)(uint64_t)>::value,
              "CRC64 setter must retain its 64-bit unsigned type");

int main() {
    VolcengineTos::TosResponse response(nullptr);
    const uint64_t crc = (std::numeric_limits<uint64_t>::max)();
    const int64_t length = (std::numeric_limits<int64_t>::max)();
    response.setHashCrc64Result(crc);
    response.setContentLength(length);
    return response.getHashCrc64Result() == crc && response.getContentLength() == length ? 0 : 1;
}
