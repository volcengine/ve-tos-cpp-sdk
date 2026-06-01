#pragma once

#include "common/Common.h"
#include "model/async/base/ChecksumBase.h"
#include "model/async/base/KeyBase.h"
#include "model/async/base/ObjectBaseOutput.h"

namespace VolcengineTos {
class ModifyObjectAsyncOutput final : public ObjectBaseOutput, public ChecksumBase, public KeyBase {
    friend class TosAsyncClient;

public:
    ModifyObjectAsyncOutput() = default;
    ~ModifyObjectAsyncOutput() override = default;

    long getNextModifyOffset() const {
        return nextModifyOffset_;
    }
    void setNextModifyOffset(const long nextModifyOffset) {
        nextModifyOffset_ = nextModifyOffset;
    }

protected:
    void headers2Output(HttpResponse& response) override {
        ObjectBaseOutput::headers2Output(response);
        ChecksumBase::headers2Output(response);
        findIntHeader(HEADER_NEXT_MODIFY_OFFSET, nextModifyOffset_);
    }

    void json2Output(json& j) override {
        ObjectBaseOutput::json2Output(j);
        ChecksumBase::json2Output(j);
        KeyBase::json2Output(j);
    }

private:
    int64_t nextModifyOffset_ = 0;
};
}  // namespace VolcengineTos
