#pragma once

#include <string>

namespace VolcengineTos {

class PiplineInputInterface {
public:
    virtual ~PiplineInputInterface() = default;

    virtual void input2Headers() {
    }

    virtual void input2Queries() {
    }

    virtual std::string input2json() {
        return "";
    }

    virtual std::string valid() {
        return "";
    }
};
}  // namespace VolcengineTos
