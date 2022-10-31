#pragma once

#include <utility>
#include "../src/external/json/json.hpp"
#include "model/RequestInfo.h"

namespace VolcengineTos {
class UploadPartCopyV2Output {
public:
    const RequestInfo& getRequestInfo() const {
        return requestInfo_;
    }
    void setRequestInfo(const RequestInfo& requestinfo) {
        requestInfo_ = requestinfo;
    }
    int getPartNumber() const {
        return partNumber_;
    }
    void setPartNumber(int partnumber) {
        partNumber_ = partnumber;
    }
    const std::string& getETag() const {
        return eTag_;
    }
    void setETag(const std::string& etag) {
        eTag_ = etag;
    }
    time_t getLastModified() const {
        return lastModified_;
    }
    void setLastModified(time_t lastmodified) {
        lastModified_ = lastmodified;
    }
    const std::string& getCopySourceVersionId() const {
        return copySourceVersionID_;
    }
    void setCopySourceVersionId(const std::string& copysourceversionid) {
        copySourceVersionID_ = copysourceversionid;
    }

    void fromJsonString(const std::string& input) {
        auto j = nlohmann::json::parse(input);
        if (j.contains("ETag"))
            j.at("ETag").get_to(eTag_);
        if (j.contains("LastModified")) {
            std::time_t lastModified =
                    TimeUtils::transLastModifiedStringToTime(j.at("LastModified").get<std::string>());
            lastModified_ = lastModified;
        }
    }

private:
    RequestInfo requestInfo_;
    int partNumber_ = 0;
    std::string eTag_;
    std::time_t lastModified_ = 0;
    std::string copySourceVersionID_;
};
}  // namespace VolcengineTos
