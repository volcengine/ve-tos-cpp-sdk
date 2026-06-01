#pragma once
#include "model/async/base/Queries.h"

#include <cstdint>
#include <string>
namespace VolcengineTos {

class OffsetBase : virtual public Queries {
public:
    OffsetBase() = default;
    ~OffsetBase() override = default;

    explicit OffsetBase(const uint64_t offset) : offset_(offset) {
    }

    uint64_t getOffset() const {
        return offset_;
    }
    void setOffset(const uint64_t offset) {
        offset_ = offset;
    }

protected:
    void input2Queries() override {
        addQuery("offset", std::to_string(offset_));
    }

private:
    uint64_t offset_ = 0;
};

}  // namespace VolcengineTos
