#pragma once

#include <string>
#include "Type.h"
namespace VolcengineTos {
class IndexDocument {
public:
    IndexDocument(std::string suffix, bool forbiddenSubDir)
            : suffix_(std::move(suffix)), forbiddenSubDir_(forbiddenSubDir) {
    }
    IndexDocument() = default;
    virtual ~IndexDocument() = default;
    const std::string& getSuffix() const {
        return suffix_;
    }
    void setSuffix(const std::string& suffix) {
        suffix_ = suffix;
    }
    bool isForbiddenSubDir() const {
        return forbiddenSubDir_;
    }
    void setForbiddenSubDir(bool forbiddenSubDir) {
        forbiddenSubDir_ = forbiddenSubDir;
    }

private:
    std::string suffix_;
    bool forbiddenSubDir_ = false;
};
}  // namespace VolcengineTos
