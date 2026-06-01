#pragma once

#include "Type.h"
#include "common/Common.h"
#include "model/async/base/Headers.h"

#include <cstdint>
#include <memory>
#include <string>

namespace VolcengineTos {
class LimiterBase : virtual public Headers {
public:
    LimiterBase() = default;
    ~LimiterBase() override = default;

    const DataTransferListener& getDataTransferListener() const {
        return dataTransferListener_;
    }
    void setDataTransferListener(const DataTransferListener& value) {
        dataTransferListener_ = value;
    }

    const std::shared_ptr<RateLimiter>& getRateLimiter() const {
        return rateLimiter_;
    }
    void setRateLimiter(const std::shared_ptr<RateLimiter>& ratelimiter) {
        rateLimiter_ = ratelimiter;
    }

    int64_t getTrafficLimit() const {
        return trafficLimit_;
    }
    void setTrafficLimit(int64_t trafficLimit) {
        trafficLimit_ = trafficLimit;
    }

protected:
    void input2Headers() override {
        if (trafficLimit_ > 0) {
            addHeader(HEADER_TRAFFIC_LIMIT, std::to_string(trafficLimit_));
        }
    }

private:
    int64_t trafficLimit_ = 0;
    DataTransferListener dataTransferListener_ = {nullptr, nullptr};  // 进度条特性
    std::shared_ptr<RateLimiter> rateLimiter_ = nullptr;              // 客户端限速
};
}  // namespace VolcengineTos
