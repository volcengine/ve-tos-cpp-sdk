#include "../src/external/json/json.hpp"

#include "model/object/UploadFileCheckpointV2.h"
#include "utils/BaseUtils.h"

#include <cstdio>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>

#ifndef _WIN32
#include <fcntl.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

namespace {
void removeFile(const std::string& filePath) {
    if (!filePath.empty()) {
        std::remove(filePath.c_str());
    }
}

#ifndef _WIN32
bool fsyncFile(const std::string& filePath) {
    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd < 0) {
        return false;
    }
    bool ok = fsync(fd) == 0;
    close(fd);
    return ok;
}

void fsyncParentDirectory(const std::string& filePath) {
    auto pos = filePath.find_last_of('/');
    std::string dir = pos == std::string::npos ? "." : filePath.substr(0, pos);
    int fd = open(dir.c_str(), O_RDONLY);
    if (fd >= 0) {
        fsync(fd);
        close(fd);
    }
}
#endif

bool replaceFile(const std::string& tempFilePath, const std::string& checkpointFilePath) {
#ifdef _WIN32
    return MoveFileExA(tempFilePath.c_str(), checkpointFilePath.c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    return std::rename(tempFilePath.c_str(), checkpointFilePath.c_str()) == 0;
#endif
}

void writeCheckpointFile(const std::string& checkpointFilePath, const std::string& content) {
    if (checkpointFilePath.empty()) {
        return;
    }

    const std::string tempFilePath = checkpointFilePath + ".tmp";
    std::ofstream ofs(tempFilePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!ofs.is_open()) {
        return;
    }

    ofs << content;
    ofs.flush();
    if (!ofs.good()) {
        ofs.close();
        removeFile(tempFilePath);
        return;
    }
    ofs.close();

#ifndef _WIN32
    if (!fsyncFile(tempFilePath)) {
        removeFile(tempFilePath);
        return;
    }
#endif

    if (!replaceFile(tempFilePath, checkpointFilePath)) {
        removeFile(tempFilePath);
        return;
    }

#ifndef _WIN32
    fsyncParentDirectory(checkpointFilePath);
#endif
}
}  // namespace

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
    writeCheckpointFile(checkpointFilePath, j.dump());
}
void VolcengineTos::UploadFileCheckpointV2::load(std::string checkpointFilePath) {
    if (checkpointFilePath.empty()) {
        return;
    }
    std::ifstream ifs(checkpointFilePath, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        return;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();
    auto str = ss.str();
    if (str.empty()) {
        removeFile(checkpointFilePath);
        return;
    }

    try {
        auto j = nlohmann::json::parse(str);
        UploadFileCheckpointV2 checkpoint;
        if (j.contains("Bucket"))
            j.at("Bucket").get_to(checkpoint.bucket_);
        if (j.contains("Key"))
            j.at("Key").get_to(checkpoint.key_);
        if (j.contains("PartSize"))
            j.at("PartSize").get_to(checkpoint.partSize_);
        if (j.contains("UploadID"))
            j.at("UploadID").get_to(checkpoint.uploadID_);
        if (j.contains("SSECustomerAlgorithm"))
            j.at("SSECustomerAlgorithm").get_to(checkpoint.sseAlgorithm_);
        if (j.contains("SSECustomerMD5"))
            j.at("SSECustomerMD5").get_to(checkpoint.sseKeyMd5_);
        if (j.contains("EncodingType"))
            j.at("EncodingType").get_to(checkpoint.encodingType_);
        if (j.contains("FilePath"))
            j.at("FilePath").get_to(checkpoint.filePath_);
        if (j.contains("FileInfo")) {
            checkpoint.fileInfo_.load(j.at("FileInfo"));
        }
        if (j.contains("PartsInfo")) {
            nlohmann::json parts = j.at("PartsInfo");
            for (auto& part : parts) {
                UploadFilePartInfoV2 ufp;
                ufp.load(part);
                checkpoint.partsInfo_.emplace_back(ufp);
            }
        }
        *this = checkpoint;
    } catch (const std::exception&) {
        removeFile(checkpointFilePath);
        *this = UploadFileCheckpointV2();
    }
}
