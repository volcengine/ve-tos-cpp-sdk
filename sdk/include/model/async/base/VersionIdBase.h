#pragma once
#include "common/Common.h"
#include "model/async/base/Headers.h"
#include "Queries.h"
#include "transport/http/HttpResponse.h"
#include "utils/BaseUtils.h"

#include <string>
#include <utility>
namespace VolcengineTos {

class VersionIdBase : virtual public Headers, virtual public Queries {
public:
    VersionIdBase() = default;
    ~VersionIdBase() override = default;

    explicit VersionIdBase(std::string vi) : versionID_(std::move(vi)) {
    }

    const std::string& getVersionId() const {
        return versionID_;
    }
    void setVersionId(const std::string& versionId) {
        versionID_ = versionId;
    }

protected:
    void headers2Output(HttpResponse& response) override {
        versionID_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_VERSIONID);
    }

    void input2Queries() override {
        addQuery("versionId", versionID_);
    }

private:
    std::string versionID_;
};

}  // namespace VolcengineTos
