#pragma once

#include "Type.h"
#include "model/async/base/BaseOutput.h"
#include "model/bucket/ListedBucket.h"
#include "model/bucket/ListedOwner.h"

#include <string>
#include <vector>

namespace VolcengineTos {
class ListBucketsAsyncOutput final : public BaseOutput {
    friend class TosAsyncClient;

public:
    ListBucketsAsyncOutput() = default;
    ~ListBucketsAsyncOutput() override = default;

    const std::vector<ListedBucket>& getBuckets() const {
        return buckets_;
    }
    const ListedOwner& getOwner() const {
        return owner_;
    }

protected:
    void json2Output(json& j) override {
        BaseOutput::json2Output(j);
        if (j.contains("Buckets")) {
            json bkts = j.at("Buckets");
            for (auto& bkt : bkts) {
                buckets_.emplace_back(parseListedBucket(bkt));
            }
        }
        if (j.contains("Owner")) {
            const json& owner = j.at("Owner");
            if (owner.contains("ID")) {
                owner_.setId(owner.at("ID").get<std::string>());
            }
            if (owner.contains("DisplayName")) {
                owner_.setDisplayName(owner.at("DisplayName").get<std::string>());
            }
        }
    }

private:
    std::vector<ListedBucket> buckets_;
    ListedOwner owner_;

    static ListedBucket parseListedBucket(const json& bucket) {
        ListedBucket lb;
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
        if (bucket.contains("BucketType"))
            lb.setBucketType(StringtoBucketType[bucket.at("BucketType").get<std::string>()]);
        return lb;
    }
};

}  // namespace VolcengineTos
