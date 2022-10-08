#pragma once
#include <string>
#include <vector>
namespace VolcengineTos {

class SourceEndpoint {
public:
    const std::vector<std::string>& getPrefix() const {
        return prefix_;
    }
    void setPrefix(const std::vector<std::string>& prefix) {
        prefix_ = prefix;
    }

private:
    std::vector<std::string> prefix_;
};

}  // namespace VolcengineTos
