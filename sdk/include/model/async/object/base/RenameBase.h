#pragma once
#include "common/Common.h"
#include "model/async/base/Headers.h"
#include "model/async/base/Queries.h"

#include <string>

namespace VolcengineTos {
class RenameBase : virtual public Headers, virtual public Queries {
public:
    RenameBase() = default;
    ~RenameBase() override = default;

    const std::string& getNewKey() const {
        return newKey_;
    }
    void setNewKey(const std::string& value) {
        newKey_ = value;
    }

    bool isRecursiveMkdir() const {
        return recursiveMkdir_;
    }
    void setRecursiveMkdir(const bool& value) {
        recursiveMkdir_ = value;
    }

    bool getForbidOverwrite() const {
        return forbidOverwrite_;
    }
    void setForbidOverwrite(bool forbidOverwrite) {
        forbidOverwrite_ = forbidOverwrite;
    }
    bool getNoReplace() const {
        return noReplace_ || forbidOverwrite_;
    }
    void setNoReplace(bool noReplace) {
        noReplace_ = noReplace;
        forbidOverwrite_ = noReplace;
    }

protected:
    void input2Headers() override {
        if (recursiveMkdir_) {
            addHeader(HEADER_RECURSIVE_MKDIR, "true");
        }
        if (getNoReplace()) {
            addHeader(HEADER_FORBID_OVERWRITE, "true");
        }
    }

    void input2Queries() override {
        addQuery("name", newKey_);
    }

private:
    std::string newKey_;
    bool noReplace_ = false;
    bool recursiveMkdir_ = false;
    bool forbidOverwrite_ = false;
};
}  // namespace VolcengineTos
