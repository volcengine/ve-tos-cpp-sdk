#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include "model/object/UploadFileCheckpointV2.h"

using namespace VolcengineTos;

namespace {
std::string checkpointPath(const std::string& name) {
#ifdef _WIN32
    const char* tempDir = std::getenv("TEMP");
    std::string dir = tempDir == nullptr ? "." : tempDir;
    return dir + "\\" + name + "-" + std::to_string(_getpid()) + ".upload";
#else
    return "/tmp/" + name + "-" + std::to_string(getpid()) + ".upload";
#endif
}

bool fileExists(const std::string& path) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    return ifs.good();
}
}  // namespace

TEST(UploadFileCheckpointV2Test, LoadCorruptCheckpointDoesNotThrowAndDeletesFile) {
    const auto path = checkpointPath("corrupt-upload-file-checkpoint-v2");
    {
        std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
        ofs << "{\"Bucket\":\"bucket\"";
    }

    UploadFileCheckpointV2 checkpoint;
    EXPECT_NO_THROW(checkpoint.load(path));
    EXPECT_FALSE(fileExists(path));
    EXPECT_FALSE(checkpoint.isValid(1, 1, "bucket", "key"));

    std::remove(path.c_str());
}

TEST(UploadFileCheckpointV2Test, LoadZeroFilledCheckpointDoesNotThrowAndDeletesFile) {
    const auto path = checkpointPath("zero-upload-file-checkpoint-v2");
    {
        std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
        ofs << std::string(32, '\0');
    }

    UploadFileCheckpointV2 checkpoint;
    EXPECT_NO_THROW(checkpoint.load(path));
    EXPECT_FALSE(fileExists(path));
    EXPECT_FALSE(checkpoint.isValid(1, 1, "bucket", "key"));

    std::remove(path.c_str());
}

TEST(UploadFileCheckpointV2Test, LoadLegacyCheckpointWrittenByOldSdk) {
    const auto path = checkpointPath("legacy-upload-file-checkpoint-v2");
    {
        std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
        ofs << R"json({
            "Bucket":"bucket",
            "EncodingType":"",
            "FileInfo":"{\"FileSize\":10,\"LastModified\":123}",
            "FilePath":"/tmp/old-source",
            "Key":"key",
            "PartSize":10,
            "PartsInfo":[
                "{\"ETag\":\"etag\",\"HashCrc64ecma\":1234,\"IsCompleted\":true,\"Offset\":0,\"PartNum\":1,\"PartSize\":10}"
            ],
            "SSECustomerAlgorithm":"",
            "SSECustomerMD5":"",
            "UploadID":"old-upload-id"
        })json";
    }

    UploadFileCheckpointV2 checkpoint;
    EXPECT_NO_THROW(checkpoint.load(path));
    EXPECT_TRUE(checkpoint.isValid(10, 123, "bucket", "key"));
    EXPECT_EQ("old-upload-id", checkpoint.getUploadId());
    ASSERT_EQ(1U, checkpoint.getPartsInfo().size());
    EXPECT_TRUE(checkpoint.getPartsInfo()[0].isCompleted());

    std::remove(path.c_str());
}

TEST(UploadFileCheckpointV2Test, DumpWritesLoadableCheckpoint) {
    const auto path = checkpointPath("valid-upload-file-checkpoint-v2");

    UploadFileInfoV2 fileInfo;
    fileInfo.setFileSize(10);
    fileInfo.setLastModified(123);

    UploadFilePartInfoV2 partInfo;
    partInfo.setPartNum(1);
    partInfo.setPartSize(10);
    partInfo.setOffset(0);
    partInfo.setIsCompleted(true);
    partInfo.setETag("etag");
    partInfo.setHashCrc64Result(1234);

    UploadFileCheckpointV2 checkpoint;
    checkpoint.setBucket("bucket");
    checkpoint.setKey("key");
    checkpoint.setUploadId("upload-id");
    checkpoint.setFileInfo(fileInfo);
    checkpoint.setPartsInfo({partInfo});
    checkpoint.dump(path);

    UploadFileCheckpointV2 loaded;
    EXPECT_NO_THROW(loaded.load(path));
    EXPECT_TRUE(loaded.isValid(10, 123, "bucket", "key"));
    EXPECT_EQ("upload-id", loaded.getUploadId());
    EXPECT_EQ(1U, loaded.getPartsInfo().size());
    EXPECT_TRUE(loaded.getPartsInfo()[0].isCompleted());

    std::remove(path.c_str());
    std::remove((path + ".tmp").c_str());
}
