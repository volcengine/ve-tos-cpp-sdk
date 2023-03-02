#include "model/object/ResumableCopyCheckpoint.h"
#include "../src/external/json/json.hpp"

using namespace nlohmann;

void VolcengineTos::ResumableCopyCheckpoint::load(const std::string& checkpointFilePath_) {
    std::ifstream ifs(checkpointFilePath_, std::ios::in);
    std::stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();
    auto str = ss.str();
    if (str.empty()) {
        return;
    }
    auto j = nlohmann::json::parse(str);
    if (j.contains("Bucket"))
        j.at("Bucket").get_to(bucket_);
    if (j.contains("Key"))
        j.at("Key").get_to(key_);
    if (j.contains("PartSize"))
        j.at("PartSize").get_to(partSize_);
    if (j.contains("UploadId"))
        j.at("UploadId").get_to(uploadID_);
    if (j.contains("CopySourceIfMatch"))
        j.at("CopySourceIfMatch").get_to(copySourceIfMatch_);
    if (j.contains("CopySourceIfModifiedSince"))
        j.at("CopySourceIfModifiedSince").get_to(copySourceIfModifiedSince_);
    if (j.contains("CopySourceIfNoneMatch"))
        j.at("CopySourceIfNoneMatch").get_to(copySourceIfNoneMatch_);
    if (j.contains("CopySourceIfUnmodifiedSince"))
        j.at("CopySourceIfUnmodifiedSince").get_to(copySourceIfUnmodifiedSince_);
    if (j.contains("CopySourceSSECAlgorithm"))
        j.at("CopySourceSSECAlgorithm").get_to(copySourceSsecAlgorithm_);
    if (j.contains("CopySourceSSECKeyMD5"))
        j.at("CopySourceSSECKeyMD5").get_to(copySourceSsecKeyMd5_);
    if (j.contains("SSECAlgorithm"))
        j.at("SSECAlgorithm").get_to(ssecAlgorithm_);
    if (j.contains("SSECKeyMD5"))
        j.at("SSECKeyMD5").get_to(ssecKeyMd5_);
    if (j.contains("EncodingType"))
        j.at("EncodingType").get_to(encodingType_);

    if (j.contains("CopySourceObjectInfo")) {
        copySourceObjectInfo_.load(j.at("CopySourceObjectInfo"));
    }
    if (j.contains("PartsInfo")) {
        nlohmann::json parts = j.at("PartsInfo");
        for (auto& part : parts) {
            ResumableCopyPartInfo rfp;
            rfp.load(part);
            partsInfo_.emplace_back(rfp);
        }
    }
}
void VolcengineTos::ResumableCopyCheckpoint::dump(const std::string& checkpointFilePath_) {
    nlohmann::json j;
    j["Bucket"] = bucket_;
    j["Key"] = key_;
    j["PartSize"] = partSize_;
    j["UploadId"] = uploadID_;
    j["IfMatch"] = copySourceIfMatch_;
    j["IfModifiedSince"] = copySourceIfModifiedSince_;
    j["IfNoneMatch"] = copySourceIfNoneMatch_;
    j["IfUnmodifiedSince"] = copySourceIfUnmodifiedSince_;
    j["CopySourceSSECAlgorithm"] = copySourceSsecAlgorithm_;
    j["CopySourceSSECKeyMD5"] = copySourceSsecKeyMd5_;
    j["SSECustomerAlgorithm"] = ssecAlgorithm_;
    j["SSECustomerMD5"] = ssecKeyMd5_;
    j["EncodingType"] = encodingType_;
    j["CopySourceObjectInfo"] = copySourceObjectInfo_.dump();
    nlohmann::json jParts = nlohmann::json::array();
    for (auto& part : partsInfo_) {
        jParts.emplace_back(part.dump());
    }
    j["PartsInfo"] = jParts;
    std::ofstream ofs(checkpointFilePath_, std::ios::out | std::ios::trunc);
    ofs << j.dump();
    ofs.close();
}
