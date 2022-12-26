#pragma once

#include <string>
#include <utility>
namespace VolcengineTos {
class Progress {
public:
    Progress() = default;
    virtual ~Progress() = default;
    Progress(double historicalObject, std::string newObject)
            : historicalObject_(historicalObject), newObject_(std::move(newObject)) {
    }
    double getHistoricalObject() const {
        return historicalObject_;
    }
    void setHistoricalObject(double historicalObject) {
        historicalObject_ = historicalObject;
    }
    const std::string& getNewObject() const {
        return newObject_;
    }
    void setNewObject(const std::string& newObject) {
        newObject_ = newObject;
    }

private:
    double historicalObject_ = 0;
    std::string newObject_;
};
}  // namespace VolcengineTos
