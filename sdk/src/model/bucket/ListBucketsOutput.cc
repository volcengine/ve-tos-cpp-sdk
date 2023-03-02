#include "model/bucket/ListBucketsOutput.h"
#include "../src/external/json/json.hpp"
#include "model/acl/Owner.h"
using namespace nlohmann;

VolcengineTos::ListedBucket parseListedBucket(const json& bucket) {
    VolcengineTos::ListedBucket lb;
    if (bucket.contains("CreationDate"))
        lb.setCreationDate(bucket.at("CreationDate").get<std::string>());
    if (bucket.contains("Name"))
        lb.setName(bucket.at("Name").get<std::string>());
    if (bucket.contains("Location"))
        lb.setLocation(bucket.at("Location").get<std::string>());
    if (bucket.contains("ExtranetEndpoint"))
        lb.setExtranetEndpoint(bucket.at("ExtranetEndpoint").get<std::string>());
    if (bucket.contains("IntranetEndpoint"))
        lb.setIntranetEndpoint(bucket.at("IntranetEndpoint").get<std::string>());
    return lb;
}

void VolcengineTos::ListBucketsOutput::fromJsonString(const std::string& output) {
    json j = json::parse(output);
    if (j.contains("Buckets")) {
        json bkts = j.at("Buckets");
        for (auto& bkt : bkts) {
            buckets_.emplace_back(parseListedBucket(bkt));
        }
    }
    if (j.at("Owner").contains("ID")) {
        owner_.setId(j.at("Owner").at("ID").get<std::string>());
    }
    if (j.at("Owner").contains("DisplayName")) {
        owner_.setDisplayName(j.at("Owner").at("DisplayName").get<std::string>());
    }
}
