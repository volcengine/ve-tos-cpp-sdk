#pragma once
#include "UploadIdBase.h"

namespace VolcengineTos {
class PartBase : public UploadIdBase {
public:
    PartBase() = default;
    ~PartBase() override = default;
    PartBase(const int& partNumber, const std::string& uploadId) : UploadIdBase(uploadId), partNumber_(partNumber) {
    }

    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(const int partNumber) {
        partNumber_ = partNumber;
    }

protected:
    std::string valid() override {
        if (partNumber_ == 0) {
            return "empty part number";
        }
        return "";
    }

    void input2Queries() override {
        UploadIdBase::input2Queries();
        addQuery("partNumber", std::to_string(partNumber_));
    }

private:
    int partNumber_ = 0;
};
}  // namespace VolcengineTos
