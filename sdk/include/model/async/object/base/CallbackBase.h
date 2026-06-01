#pragma once

#include "common/Common.h"
#include "model/async/base/Headers.h"

#include <string>

namespace VolcengineTos {
class CallbackBase : virtual public Headers {
public:
    CallbackBase() = default;
    ~CallbackBase() override = default;

    const std::string& getCallBack() const {
        return callBack_;
    }
    void setCallBack(const std::string& callBack) {
        callBack_ = callBack;
    }
    const std::string& getCallBackVar() const {
        return callBackVar_;
    }
    void setCallBackVar(const std::string& callBackVar) {
        callBackVar_ = callBackVar;
    }

protected:
    void input2Headers() override {
        addHeader(HEADER_CALLBACK, callBack_);
        addHeader(HEADER_CALLBACK_VAR, callBackVar_);
    }

private:
    std::string callBack_;
    std::string callBackVar_;
};
}  // namespace VolcengineTos
