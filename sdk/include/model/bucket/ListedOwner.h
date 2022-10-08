
#pragma once

#include <string>
namespace VolcengineTos {
class ListedOwner {
public:
    const std::string& getId() const {
        return id_;
    }
    void setId(const std::string& id) {
        id_ = id;
    }
    const std::string& getDisplayName() const {
        return displayName_;
    }
    void setDisplayName(const std::string& displayName) {
        displayName_ = displayName;
    }

private:
    std::string id_;
    std::string displayName_;
};
}  // namespace VolcengineTos
