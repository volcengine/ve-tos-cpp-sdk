#pragma once

#include <string>
namespace VolcengineTos {
class ListedBucket {
public:
    const std::string& getCreationDate() const {
        return creationDate_;
    }
    const std::string& getName() const {
        return name_;
    }
    const std::string& getLocation() const {
        return location_;
    }
    const std::string& getExtranetEndpoint() const {
        return extranetEndpoint_;
    }
    const std::string& getIntranetEndpoint() const {
        return intranetEndpoint_;
    }

    void setCreationDate(const std::string& creationDate) {
        creationDate_ = creationDate;
    }
    void setName(const std::string& name) {
        name_ = name;
    }
    void setLocation(const std::string& location) {
        location_ = location;
    }
    void setExtranetEndpoint(const std::string& extranetEndpoint) {
        extranetEndpoint_ = extranetEndpoint;
    }
    void setIntranetEndpoint(const std::string& intranetEndpoint) {
        intranetEndpoint_ = intranetEndpoint;
    }

private:
    std::string creationDate_;
    std::string name_;
    std::string location_;
    std::string extranetEndpoint_;
    std::string intranetEndpoint_;
};
}  // namespace VolcengineTos
