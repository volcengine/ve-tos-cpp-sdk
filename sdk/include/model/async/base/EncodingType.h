#pragma once

#include "model/async/base/PiplineOutputInterface.h"
#include "model/async/base/Queries.h"

#include <string>

namespace VolcengineTos {
class EncodingType : virtual public Queries, virtual public PiplineOutputInterface {
public:
    EncodingType() = default;
    ~EncodingType() override = default;

    const std::string& getEncodingType() const {
        return encodingType_;
    }
    void setEncodingType(const std::string& encodingType) {
        encodingType_ = encodingType;
    }

protected:
    void input2Queries() override {
        addQuery("encoding-type", encodingType_);
    }

    void json2Output(json& j) override {
        if (j.contains("EncodingType")) {
            encodingType_ = j["EncodingType"].get<std::string>();
        }
    }

private:
    std::string encodingType_;
};
}  // namespace VolcengineTos
