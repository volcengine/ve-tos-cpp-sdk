#pragma once
#include "common/Common.h"
#include "BaseOutput.h"
#include "utils/BaseUtils.h"

#include <string>
namespace VolcengineTos {

class ObjectBaseOutput : public BaseOutput {
public:
    ObjectBaseOutput() = default;
    ~ObjectBaseOutput() override = default;

    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& value) {
        versionID_ = value;
    }
    const std::string& getContentRange() const {
        return contentRange_;
    }
    void setContentRange(const std::string& value) {
        contentRange_ = value;
    }
    bool getDeleteMarker() const {
        return deleteMarker_;
    }
    void setDeleteMarker(const bool value) {
        deleteMarker_ = value;
    }
    std::string getObjectType() const {
        return objectType_;
    }
    void setObjectType(const std::string& value) {
        objectType_ = value;
    }
    bool isDirectory() const {
        return isDirectory_;
    }
    void setIsDirectory(const bool value) {
        isDirectory_ = value;
    }

protected:
    void headers2Output(HttpResponse& response) override {
        BaseOutput::headers2Output(response);

        versionID_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_VERSIONID);
        contentRange_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), http::HEADER_CONTENT_RANGE);

        deleteMarker_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_DELETE_MARKER) == "true";
        objectType_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_OBJECT_TYPE);
        const auto hashCrc64ecmaString = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_CRC64);
        isDirectory_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_DIRECTORY) == "true";
    }

private:
    std::string versionID_;
    std::string contentRange_;

    bool deleteMarker_ = false;
    std::string objectType_;
    bool isDirectory_ = false;
};

}  // namespace VolcengineTos
