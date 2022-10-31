#pragma once

#include <string>
namespace VolcengineTos {
class Condition {
public:
    Condition() = default;
    virtual ~Condition() = default;
    explicit Condition(int httpCode) : httpCode_(httpCode) {
    }
    int getHttpCode() const {
        return httpCode_;
    }
    void setHttpCode(int httpCode) {
        httpCode_ = httpCode;
    }

private:
    int httpCode_ = 0;
};
}  // namespace VolcengineTos
