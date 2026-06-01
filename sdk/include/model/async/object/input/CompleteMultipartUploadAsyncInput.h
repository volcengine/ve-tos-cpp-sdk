#pragma once

#include "PutObjectAsyncInput.h"
#include "model/async/object/base/UploadIdBase.h"
#include "model/object/UploadedPartV2.h"

#include <utility>

namespace VolcengineTos {
class CompleteMultipartUploadAsyncInput final : public PutObjectAsyncInput, public UploadIdBase {
    friend class TosAsyncClient;

public:
    CompleteMultipartUploadAsyncInput() = delete;
    ~CompleteMultipartUploadAsyncInput() override = default;
    CompleteMultipartUploadAsyncInput(const std::string& bucket, const std::string& key, const std::string& uploadId)
            : PutObjectAsyncInput(bucket, key, TransferEncoding::ContentLength), UploadIdBase(uploadId) {
    }

    const std::vector<UploadedPartV2>& getParts() const {
        return parts_;
    }
    void setParts(const std::vector<UploadedPartV2>& parts) {
        parts_ = parts;
    }
    bool isCompleteAll() const {
        return completeAll_;
    }
    void setCompleteAll(bool completeAll) {
        completeAll_ = completeAll;
    }

protected:
    void input2Queries() override {
        PutObjectAsyncInput::input2Queries();
        UploadIdBase::input2Queries();
    }

    void input2Headers() override {
        PutObjectAsyncInput::input2Headers();
        if (completeAll_) {
            addHeader(HEADER_COMPLETE_ALL, "yes");
        }
    }

    std::string valid() override {
        std::string error_string = PutObjectAsyncInput::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        error_string = UploadIdBase::valid();
        if (!error_string.empty()) {
            return error_string;
        }

        if (completeAll_) {
            if (!parts_.empty()) {
                return "tos: Parameter Parts is not empty when use complete all";
            }
        } else {
            if (parts_.empty()) {
                return "tos: Parameter Parts is not set";
            }
        }
        return "";
    }

    std::string input2json() override {
        if (completeAll_) {
            return "";
        }

        nlohmann::json j;
        nlohmann::json partsArray = nlohmann::json::array();
        for (auto& p : parts_) {
            nlohmann::json parts;
            auto partNumber_ = std::to_string(p.getPartNumber());
            if (!partNumber_.empty())
                parts["PartNumber"] = p.getPartNumber();
            if (!p.getETag().empty())
                parts["ETag"] = p.getETag();
            partsArray.push_back(parts);
        }
        if (!partsArray.empty())
            j["Parts"] = partsArray;
        return j.dump();
    }

    void json2Output(json& j) override {};

private:
    std::vector<UploadedPartV2> parts_;
    bool completeAll_ = false;
};
}  // namespace VolcengineTos
