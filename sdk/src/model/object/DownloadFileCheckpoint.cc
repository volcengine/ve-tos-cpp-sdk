#include "model/object/DownloadFileCheckpoint.h"
#include "../src/external/json/json.hpp"

void VolcengineTos::DownloadFileCheckpoint::load(const std::string& checkpointFilePath_) {
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
    if (j.contains("VersionID"))
        j.at("VersionID").get_to(versionID_);
    if (j.contains("PartSize"))
        j.at("PartSize").get_to(partSize_);
    if (j.contains("IfMatch"))
        j.at("IfMatch").get_to(ifMatch_);
    if (j.contains("IfModifiedSince"))
        j.at("IfModifiedSince").get_to(ifModifiedSince_);
    if (j.contains("IfNoneMatch"))
        j.at("IfNoneMatch").get_to(ifNoneMatch_);
    if (j.contains("IfUnmodifiedSince"))
        j.at("IfUnmodifiedSince").get_to(ifUnmodifiedSince_);
    if (j.contains("SSEAlgorithm"))
        j.at("SSEAlgorithm").get_to(sseAlgorithm_);
    if (j.contains("SSECustomerMD5"))
        j.at("SSECustomerMD5").get_to(sseKeyMd5_);
    if (j.contains("ObjectInfo")) {
        objectInfo_.load(j.at("ObjectInfo"));
    }
    if (j.contains("FileInfo")) {
        fileInfo_.load(j.at("FileInfo"));
    }
    if (j.contains("PartsInfo")) {
        nlohmann::json parts = j.at("PartsInfo");
        for (auto& part : parts) {
            DownloadFilePartInfo dfp;
            dfp.load(part);
            partsInfo_.emplace_back(dfp);
        }
    }
}
void VolcengineTos::DownloadFileCheckpoint::dump(const std::string& checkpointFilePath_) {
    nlohmann::json j;
    j["Bucket"] = bucket_;
    j["Key"] = key_;
    j["VersionID"] = versionID_;
    j["PartSize"] = partSize_;
    j["IfMatch"] = ifMatch_;
    j["IfModifiedSince"] = ifModifiedSince_;
    j["IfNoneMatch"] = ifNoneMatch_;
    j["IfUnmodifiedSince"] = ifUnmodifiedSince_;

    j["SSECustomerAlgorithm"] = sseAlgorithm_;
    j["SSECustomerMD5"] = sseKeyMd5_;
    j["ObjectInfo"] = objectInfo_.dump();
    j["FileInfo"] = fileInfo_.dump();
    nlohmann::json jParts = nlohmann::json::array();
    for (auto& part : partsInfo_) {
        jParts.emplace_back(part.dump());
    }
    j["PartsInfo"] = jParts;
    std::ofstream ofs(checkpointFilePath_, std::ios::out | std::ios::trunc);
    ofs << j.dump();
    ofs.close();
}
