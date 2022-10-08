#pragma once

#include <string>
#include <utility>
#include <vector>
#include <ostream>
#include <algorithm>
#include "../src/external/json/json.hpp"
#include "UploadedPartV2.h"
namespace VolcengineTos {

class CompleteMultipartUploadV2Input {
public:
    CompleteMultipartUploadV2Input(std::string bucket, std::string key, std::string uploadid,
                                   std::vector<UploadedPartV2> parts)
            : bucket_(std::move(bucket)),
              key_(std::move(key)),
              uploadID_(std::move(uploadid)),
              parts_(std::move(parts)) {
    }
    CompleteMultipartUploadV2Input() = default;
    ~CompleteMultipartUploadV2Input() = default;

    const std::string& getBucket() const {
        return bucket_;
    }
    void setBucket(const std::string& bucket) {
        bucket_ = bucket;
    }
    const std::string& getKey() const {
        return key_;
    }
    void setKey(const std::string& key) {
        key_ = key;
    }
    const std::string& getUploadId() const {
        return uploadID_;
    }
    void setUploadId(const std::string& uploadId) {
        uploadID_ = uploadId;
    }
    const std::vector<UploadedPartV2>& getParts() const {
        return parts_;
    }
    void setParts(const std::vector<UploadedPartV2>& parts) {
        parts_ = parts;
    }
    std::string toJsonString() const {
        nlohmann::json j;
        nlohmann::json partsArray = nlohmann::json::array();
        for (auto& p : parts_) {
            nlohmann::json parts;
            auto partNumber_ = std::to_string(p.getPartNumber());
            if (!partNumber_.empty())
                parts["PartNumber"] = p.getPartNumber();
            if (!p.getETag().empty())
                parts["ETag"] = p.getETag();
            partsArray.push_back(parts);
        }
        if (!partsArray.empty())
            j["Parts"] = partsArray;
        return j.dump();
    }

private:
    std::string bucket_;
    std::string key_;
    std::string uploadID_;
    std::vector<UploadedPartV2> parts_;
};

}  // namespace VolcengineTos
