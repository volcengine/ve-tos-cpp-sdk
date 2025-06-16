#pragma once
#include <ctime>
namespace VolcengineTos {

class GenericInput {
public:
    GenericInput() : requestDate_(0) {}
    ~GenericInput() = default;

    explicit GenericInput(std::time_t requestDate) : requestDate_(requestDate) {
    }

    void setRequestDate(std::time_t requestDate) {
        requestDate_ = requestDate;
    }
    std::time_t getRequestDate() const {
        return requestDate_;
    }
private:
    std::time_t requestDate_ = 0;
};
}  // namespace VolcengineTos
