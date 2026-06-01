#pragma once

#include "common/Common.h"
#include "model/async/base/BaseOutput.h"
#include "model/async/base/ChecksumBase.h"

namespace VolcengineTos {

class AppendObjectAsyncOutput : public BaseOutput, public ChecksumBase {
    friend class TosAsyncClient;

public:
    AppendObjectAsyncOutput() = default;
    ~AppendObjectAsyncOutput() override = default;

    u_int64_t getNextAppendOffset() const {
        return nextAppendOffset_;
    }
    void setNextAppendOffset(const u_int64_t nextAppendOffset) {
        nextAppendOffset_ = nextAppendOffset;
    }

protected:
    void json2Output(json& j) override {
        BaseOutput::json2Output(j);
        ChecksumBase::json2Output(j);
    }

    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);
        ChecksumBase::headers2Output(response);
        findUintHeader(HEADER_NEXT_APPEND_OFFSET, nextAppendOffset_);
    }

private:
    u_int64_t nextAppendOffset_ = 0;
};

}  // namespace VolcengineTos
