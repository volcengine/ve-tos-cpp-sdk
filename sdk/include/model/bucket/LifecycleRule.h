#pragma once

#include <string>
#include "Condition.h"
#include "Redirect.h"
namespace VolcengineTos {
class LifecycleRule {
public:
    const std::string& getId() const {
        return id_;
    }
    void setId(const std::string& id) {
        id_ = id;
    }

private:
    std::string id_;
    std::string prefix_;
    std::string status;
};
}  // namespace VolcengineTos
