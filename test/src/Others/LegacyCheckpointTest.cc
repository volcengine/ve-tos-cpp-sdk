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

#include "model/object/DownloadFileCheckpoint.h"
#include "model/object/ResumableCopyCheckpoint.h"
#include "model/object/UploadFileCheckpoint.h"

using namespace VolcengineTos;

namespace {
std::string checkpointPath(const std::string& name) {
#ifdef _WIN32
    const char* tempDir = std::getenv("TEMP");
    std::string dir = tempDir == nullptr ? "." : tempDir;
    return dir + "\\" + name + "-" + std::to_string(_getpid()) + ".checkpoint";
#else
    return "/tmp/" + name + "-" + std::to_string(getpid()) + ".checkpoint";
#endif
}

void writeCheckpoint(const std::string& path, const std::string& content) {
    std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
    ofs << content;
}

bool fileExists(const std::string& path) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    return ifs.good();
}
}  // namespace

TEST(LegacyCheckpointTest, UploadLoadCorruptJsonDoesNotThrow) {
    const auto path = checkpointPath("corrupt-upload");
    writeCheckpoint(path, R"json({"Bucket":"bucket")json");

    UploadFileCheckpoint checkpoint;
    checkpoint.setCheckpointFilePath(path);
    EXPECT_NO_THROW(checkpoint.load());
    EXPECT_FALSE(fileExists(path));
    EXPECT_TRUE(checkpoint.getBucket().empty());
    EXPECT_EQ(path, checkpoint.getCheckpointFilePath());
}

TEST(LegacyCheckpointTest, DownloadLoadWrongFieldTypeDoesNotThrow) {
    const auto path = checkpointPath("wrong-type-download");
    writeCheckpoint(path, R"json({"Bucket":123})json");

    DownloadFileCheckpoint checkpoint;
    EXPECT_NO_THROW(checkpoint.load(path));
    EXPECT_FALSE(fileExists(path));
    EXPECT_TRUE(checkpoint.getBucket().empty());
}

TEST(LegacyCheckpointTest, ResumableCopyLoadCorruptJsonDoesNotThrow) {
    const auto path = checkpointPath("corrupt-resumable-copy");
    writeCheckpoint(path, R"json({"UploadId":"upload-id")json");

    ResumableCopyCheckpoint checkpoint;
    EXPECT_NO_THROW(checkpoint.load(path));
    EXPECT_FALSE(fileExists(path));
    EXPECT_TRUE(checkpoint.getUploadId().empty());
}
