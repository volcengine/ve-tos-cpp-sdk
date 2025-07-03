#pragma once
#include <ctime>
#include <map>
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

    const std::map<std::string, std::string>& getRequestHeader() const {
        return requestHeader_;
    }
    void setRequestHeader(const std::map<std::string, std::string>& requestHeader) {
        requestHeader_ = requestHeader;
    }

private:
    std::time_t requestDate_ = 0;
    std::map<std::string, std::string> requestHeader_ = {};
};
}  // namespace VolcengineTos