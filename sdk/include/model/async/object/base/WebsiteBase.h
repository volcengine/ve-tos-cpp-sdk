#pragma once
#include "common/Common.h"
#include "model/async/base/Headers.h"

namespace VolcengineTos {
class WebsiteBase : virtual public Headers {
public:
    WebsiteBase() = default;
    ~WebsiteBase() override = default;

    const std::string& getWebsiteRedirectLocation() const {
        return websiteRedirectLocation_;
    }
    void setWebsiteRedirectLocation(const std::string& value) {
        websiteRedirectLocation_ = value;
    }

protected:
    void input2Headers() override {
        addHeader(HEADER_WEBSITE_REDIRECT_LOCATION, websiteRedirectLocation_);
    }

    void headers2Output(HttpResponse& response) override {
        websiteRedirectLocation_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_WEBSITE_REDIRECT_LOCATION);
    }

private:
    std::string websiteRedirectLocation_;
};
}  // namespace VolcengineTos
