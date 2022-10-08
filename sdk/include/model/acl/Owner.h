#pragma once

#include <string>
namespace VolcengineTos {
class Owner {
public:
    const std::string& getId() const {
        return id_;
    }
    void setId(const std::string& id) {
        Owner::id_ = id;
    }
    const std::string& getDisplayName() const {
        return displayName_;
    }
    void setDisplayName(const std::string& displayName) {
        Owner::displayName_ = displayName;
    }

private:
    std::string id_;
    std::string displayName_;
};
}  // namespace VolcengineTos
