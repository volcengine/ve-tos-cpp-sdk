#pragma once

#include "model/async/base/ChecksumBase.h"
#include "model/async/base/KeyBase.h"
#include "model/async/base/ObjectBaseOutput.h"
#include "model/async/object/base/UserMetaBase.h"

#include <cstdint>
#include <map>
#include <string>

namespace VolcengineTos {
class GetFileStatusAsyncOutput final
    : public ObjectBaseOutput,
      public ChecksumBase,
      public KeyBase,
      public UserMetaBase {
    friend class TosAsyncClient;

public:
    GetFileStatusAsyncOutput() = default;
    ~GetFileStatusAsyncOutput() override = default;

    int64_t getSize() const {
        return size_;
    }
    void setSize(const int64_t& value) {
        size_ = value;
    }

protected:
    void input2Headers() override {
    }

    void headers2Output(HttpResponse& response) override {
        ObjectBaseOutput::headers2Output(response);
        ChecksumBase::headers2Output(response);
        UserMetaBase::headers2Output(response);
    }

    void json2Output(json& j) override {
        ObjectBaseOutput::json2Output(j);
        ChecksumBase::json2Output(j);
        KeyBase::json2Output(j);

        if (j.contains("Size")) {
            size_ = j["Size"].get<int64_t>();
        }
        if (j.contains("Type")) {
            setObjectType(j["Type"].get<std::string>());
            if (j["Type"].get<std::string>() == "Directory") {
                setIsDirectory(true);
            }
        }
        if (j.contains("UserMeta") && j["UserMeta"].is_array()) {
            std::map<std::string, std::string> meta;
            for (const auto& item : j["UserMeta"]) {
                if (item.contains("Key") && item.contains("Value")) {
                    meta[item.at("Key").get<std::string>()] = item.at("Value").get<std::string>();
                }
            }
            setMeta(meta);
        }
    }

private:
    int64_t size_ = 0;
};
}  // namespace VolcengineTos
