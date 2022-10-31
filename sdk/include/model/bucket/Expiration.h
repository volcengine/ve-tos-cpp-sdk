#pragma once

#include <string>
#include "Type.h"

namespace VolcengineTos {
class Expiration {
public:
    explicit Expiration(time_t date) : date_(date) {
    }
    explicit Expiration(int days) : days_(days) {
    }
    Expiration() = default;
    virtual ~Expiration() = default;
    time_t getDate() const {
        return date_;
    }
    void setDate(time_t date) {
        date_ = date;
    }
    int getDays() const {
        return days_;
    }
    void setDays(int days) {
        days_ = days;
    }

private:
    std::time_t date_ = 0;
    int days_ = 0;
};
}  // namespace VolcengineTos
