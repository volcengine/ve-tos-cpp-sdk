#pragma once
#include "model/async/base/Queries.h"
#include "utils/BaseUtils.h"

#include <ctime>
#include <string>

namespace VolcengineTos {
class UserResponseBase : virtual public Queries {
public:
    UserResponseBase() = default;
    ~UserResponseBase() override = default;

    const std::string& getResponseCacheControl() const {
        return responseCacheControl_;
    }
    void setResponseCacheControl(const std::string& value) {
        responseCacheControl_ = value;
    }
    const std::string& getResponseContentDisposition() const {
        return responseContentDisposition_;
    }
    void setResponseContentDisposition(const std::string& value) {
        responseContentDisposition_ = value;
    }
    const std::string& getResponseContentEncoding() const {
        return responseContentEncoding_;
    }
    void setResponseContentEncoding(const std::string& value) {
        responseContentEncoding_ = value;
    }
    const std::string& getResponseContentLanguage() const {
        return responseContentLanguage_;
    }
    void setResponseContentLanguage(const std::string& value) {
        responseContentLanguage_ = value;
    }
    const std::string& getResponseContentType() const {
        return responseContentType_;
    }
    void setResponseContentType(const std::string& value) {
        responseContentType_ = value;
    }
    time_t getResponseExpires() const {
        return responseExpires_;
    }
    void setResponseExpires(time_t value) {
        responseExpires_ = value;
    }

protected:
    void input2Queries() override {
        addQuery("response-cache-control", responseCacheControl_);
        addQuery("response-content-disposition", responseContentDisposition_);
        addQuery("response-content-encoding", responseContentEncoding_);
        addQuery("response-content-language", responseContentLanguage_);
        addQuery("response-content-type", responseContentType_);
        addQuery("response-expires", TimeUtils::transTimeToGmtTime(responseExpires_));
    }

private:
    std::string responseCacheControl_;
    std::string responseContentDisposition_;
    std::string responseContentEncoding_;
    std::string responseContentLanguage_;
    std::string responseContentType_;
    std::time_t responseExpires_ = 0;
};
}  // namespace VolcengineTos
