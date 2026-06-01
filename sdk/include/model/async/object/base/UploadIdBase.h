#pragma once
#include "model/async/base/PiplineOutputInterface.h"
#include "model/async/base/Queries.h"

#include <string>
#include <utility>

namespace VolcengineTos {
class UploadIdBase : virtual public Queries, virtual public PiplineOutputInterface {
public:
    UploadIdBase() = default;
    ~UploadIdBase() override = default;
    explicit UploadIdBase(std::string uploadId) : uploadID_(std::move(uploadId)) {
    }

    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }

protected:
    std::string valid() override {
        if (uploadID_.empty()) {
            return "empty upload id";
        }
        return "";
    }

    void input2Queries() override {
        addQuery("uploadId", uploadID_);
    }

    void json2Output(json& j) override {
        if (j.contains("UploadId")) {
            uploadID_ = j["UploadId"].get<std::string>();
        }
    }

private:
    std::string uploadID_;
};
}  // namespace VolcengineTos
