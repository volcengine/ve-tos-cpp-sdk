#pragma once

#include <string>
#include "Type.h"

namespace VolcengineTos {
class Transition {
public:
    Transition(time_t date, StorageClassType storageClass) : date_(date), storageClass_(storageClass) {
    }
    Transition(int days, StorageClassType storageClass) : days_(days), storageClass_(storageClass) {
    }
    Transition() = default;
    virtual ~Transition() = default;
    time_t getDate() const {
        return date_;
    }
    void setDate(time_t date) {
        date_ = date;
    }
    int getDays() const {
        return days_;
    }
    void setDays(int days) {
        days_ = days;
    }
    StorageClassType getStorageClass() const {
        return storageClass_;
    }
    void setStorageClass(StorageClassType storageClass) {
        storageClass_ = storageClass;
    }
    const std::string& getStringFormatStorageClass() const {
        return StorageClassTypetoString[storageClass_];
    }

private:
    std::time_t date_ = 0;
    int days_ = 0;
    StorageClassType storageClass_ = StorageClassType::NotSet;
};
}  // namespace VolcengineTos
