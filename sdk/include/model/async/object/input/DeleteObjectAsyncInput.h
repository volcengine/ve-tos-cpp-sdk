#pragma once

#include "model/async/base/BaseHttp.h"
#include "model/async/base/ObjectBaseInput.h"

#include <string>

namespace VolcengineTos {
class DeleteObjectAsyncInput final : public BaseHttp, public ObjectBaseInput {
    friend class TosAsyncClient;

public:
    DeleteObjectAsyncInput() = delete;
    ~DeleteObjectAsyncInput() override = default;
    DeleteObjectAsyncInput(const std::string& bucket, const std::string& key) : ObjectBaseInput(bucket, key) {
    }

    bool getRecursive() const {
        return recursive_;
    }

    void setRecursive(const bool isRecursive) {
        recursive_ = isRecursive;
    }

    bool getSkipTrash() const {
        return skipTrash_;
    }

    void setSkipTrash(const bool skipTrash) {
        skipTrash_ = skipTrash;
    }

protected:
    void input2Headers() override {
        BaseHttp::input2Headers();
        ObjectBaseInput::input2Headers();
    }

    void input2Queries() override {
        ObjectBaseInput::input2Queries();

        if (recursive_) {
            addQuery("recursive", "true");
        }
        if (skipTrash_) {
            addQuery("skipTrash", "true");
        }
    }

    void headers2Output(HttpResponse& response) override {
    }

private:
    bool recursive_ = false;
    bool skipTrash_ = false;
};
}  // namespace VolcengineTos
