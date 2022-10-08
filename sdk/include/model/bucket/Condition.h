#pragma once

#include <string>
namespace VolcengineTos {
class Condition {
public:
    int getHttpCode() const {
        return httpCode_;
    }
    void setHttpCode(int httpCode) {
        httpCode_ = httpCode;
    }

private:
    int httpCode_;
};
}  // namespace VolcengineTos
