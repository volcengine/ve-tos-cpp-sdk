#pragma once

#include <string>

namespace VolcengineTos {
class SchemeHostParameter {
public:
    std::string scheme_;
    std::string host_;
    int urlMode_ = 0;
};

}  // namespace VolcengineTos
