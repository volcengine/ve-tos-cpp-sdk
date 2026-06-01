#pragma once
#include "Headers.h"

namespace VolcengineTos {
class AreaBase : virtual public Headers {
public:
    AreaBase() = default;
    ~AreaBase() override = default;

    const AzRedundancyType& getAzRedundancy() const {
        return azRedundancy_;
    }
    void setAzRedundancy(const AzRedundancyType& azRedundancy) {
        azRedundancy_ = azRedundancy;
    }

    const std::string& getRegion() const {
        return region_;
    }

protected:
    void input2Headers() override {
        addHeader(HEADER_AZ_REDUNDANCY, AzRedundancyTypetoString[azRedundancy_]);
    }

    void headers2Output(HttpResponse& response) override {
        region_ = MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_BUCKET_REGION);
        azRedundancy_ =
                StringtoAzRedundancyType[MapUtils::findValueByKeyIgnoreCase(getHeaders(), HEADER_BUCKET_REGION)];
    }

private:
    AzRedundancyType azRedundancy_ = AzRedundancyType::NotSet;

    // output
    std::string region_;
};
}  // namespace VolcengineTos
