#pragma once

#include "model/async/base/ObjectBaseOutput.h"

namespace VolcengineTos {

class DeleteObjectAsyncOutput final : public ObjectBaseOutput {
    friend class TosAsyncClient;

public:
    DeleteObjectAsyncOutput() = default;
    ~DeleteObjectAsyncOutput() override = default;
    const std::string& getTrashPath() const {
        return trashPath_;
    }
    void setTrashPath(const std::string& trashPath) {
        trashPath_ = trashPath;
    }

protected:
    void headers2Output(HttpResponse& response) override {
        ObjectBaseOutput::headers2Output(response);
        trashPath_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_Trash_Path);
    }

private:
    std::string trashPath_;
};

}  // namespace VolcengineTos