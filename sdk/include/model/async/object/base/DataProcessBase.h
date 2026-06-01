#pragma once

#include <string>
#include "model/async/base/Queries.h"

namespace VolcengineTos {
class DataProcessBase : virtual public Queries {
public:
    DataProcessBase() = default;
    ~DataProcessBase() override = default;

    const std::string& getProcess() const {
        return process_;
    }
    void setProcess(const std::string& process) {
        process_ = process;
    }

protected:
    void input2Queries() override {
        addQuery("x-tos-process", process_);
    }

private:
    std::string process_;
};
}  // namespace VolcengineTos
