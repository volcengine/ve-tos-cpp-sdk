#include "../src/external/json/json.hpp"

#include "model/object/UploadFileCheckpointV2.h"
#include "utils/BaseUtils.h"

void VolcengineTos::UploadFileCheckpointV2::dump(std::string checkpointFilePath) {
    nlohmann::json j;
    j["Bucket"] = bucket_;
    j["Key"] = key_;
    j["PartSize"] = partSize_;
    j["UploadID"] = uploadID_;
    j["SSECustomerAlgorithm"] = sseAlgorithm_;
    j["SSECustomerMD5"] = sseKeyMd5_;
    j["EncodingType"] = encodingType_;
    j["FilePath"] = FileUtils::stringToUTF8(filePath_);
    j["FileInfo"] = fileInfo_.dump();
    nlohmann::json jParts = nlohmann::json::array();
    for (auto& part : partsInfo_) {
        jParts.emplace_back(part.dump());
    }
    j["PartsInfo"] = jParts;
    std::ofstream ofs(checkpointFilePath, std::ios::out | std::ios::trunc);
    ofs << j.dump();
    ofs.close();
}
void VolcengineTos::UploadFileCheckpointV2::load(std::string checkpointFilePath) {
    if (checkpointFilePath.empty()) {
        return;
    }
    std::ifstream ifs(checkpointFilePath, std::ios::in);
    std::stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();
    auto str = ss.str();
    if (str.empty()) {
        return;
    }
    auto j = nlohmann::json::parse(ss.str());
    if (j.contains("Bucket"))
        j.at("Bucket").get_to(bucket_);
    if (j.contains("Key"))
        j.at("Key").get_to(key_);
    if (j.contains("PartSize"))
        j.at("PartSize").get_to(partSize_);
    if (j.contains("UploadID"))
        j.at("UploadID").get_to(uploadID_);
    if (j.contains("SSECustomerAlgorithm"))
        j.at("SSECustomerAlgorithm").get_to(sseAlgorithm_);
    if (j.contains("SSECustomerMD5"))
        j.at("SSECustomerMD5").get_to(sseKeyMd5_);
    if (j.contains("EncodingType"))
        j.at("EncodingType").get_to(encodingType_);
    if (j.contains("FilePath"))
        j.at("FilePath").get_to(filePath_);
    if (j.contains("FileInfo")) {
        fileInfo_.load(j.at("FileInfo"));
    }
    if (j.contains("PartsInfo")) {
        nlohmann::json parts = j.at("PartsInfo");
        for (auto& part : parts) {
            UploadFilePartInfoV2 ufp;
            ufp.load(part);
            partsInfo_.emplace_back(ufp);
        }
    }
}
