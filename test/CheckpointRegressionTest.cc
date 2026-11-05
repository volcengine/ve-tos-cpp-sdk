#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <random>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include "external/json/json.hpp"
#include "model/object/DownloadFileCheckpoint.h"
#include "model/object/ResumableCopyCheckpoint.h"
#include "model/object/UploadFileCheckpointV2.h"
#include "utils/BaseUtils.h"
#include "utils/CheckpointJson.h"

using namespace VolcengineTos;

std::string getCheckpointPath(const std::string&, const std::string&, const std::string&, const std::string&);
std::string getDownloadCheckpointPath(const std::string&, const std::string&, const std::string&, const std::string&,
                                      const std::string&);
std::string getCheckpointPath(const ResumableCopyObjectInput&);

namespace {
using nlohmann::json;

std::string readFile(const std::string& path) {
    std::ifstream stream(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

void writeFile(const std::string& path, const std::string& content) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    stream << content;
}

std::vector<std::string> stringSamples() {
    std::string allBytes;
    for (unsigned int byte = 0; byte < 256; ++byte) {
        allBytes.push_back(static_cast<char>(byte));
    }
    return {"ascii",
            "\xe4\xb8\xad\xe6\x96\x87-\xf0\x9f\x98\x80",
            std::string("nul\0end", 7),
            "__tos_checkpoint_bytes_v1",
            "\xc1",
            "\xe4\xb8",
            "\xc0\xaf",
            "\xed\xa0\x80",
            "\xf4\x90\x80\x80",
            allBytes};
}

class CheckpointRegressionTest : public ::testing::Test {
protected:
    std::string root;
    std::vector<std::string> files;
    std::vector<std::string> directories;

    void SetUp() override {
#ifdef _WIN32
        const auto pid = _getpid();
        const char* temp = std::getenv("TEMP");
        root = std::string(temp == nullptr ? "." : temp);
#else
        const auto pid = getpid();
        root = "/tmp";
#endif
        root += TOS_PATH_DELIMITER;
        root += "tos-checkpoint-regression-" + std::to_string(pid) + "-" +
                ::testing::UnitTest::GetInstance()->current_test_info()->name();
        ASSERT_TRUE(FileUtils::CreateDir(root, false));
        directories.push_back(root);
    }

    void TearDown() override {
        for (const auto& file : files) {
            std::remove(file.c_str());
            std::remove((file + ".tmp").c_str());
        }
        for (auto it = directories.rbegin(); it != directories.rend(); ++it) {
#ifdef _WIN32
            _rmdir(it->c_str());
#else
            rmdir(it->c_str());
#endif
        }
    }

    std::string file(const std::string& name) {
        files.push_back(root + TOS_PATH_DELIMITER + name);
        return files.back();
    }

    std::string resolve(int operation, const std::string& checkpoint) {
        if (operation == 0) {
            return getCheckpointPath("test-bucket", "key", checkpoint, "source");
        }
        if (operation == 1) {
            return getDownloadCheckpointPath("test-bucket", "key", "", checkpoint, "destination");
        }
        ResumableCopyObjectInput input;
        input.setBucket("test-bucket");
        input.setKey("key");
        input.setSrcBucket("source-bucket");
        input.setSrcKey("source");
        input.setCheckpointFile(checkpoint);
        return getCheckpointPath(input);
    }
};

TEST_F(CheckpointRegressionTest, UploadStringsRoundTripWithoutByteLoss) {
    const auto path = file("upload.checkpoint");
    size_t sample = 0;
    for (const auto& value : stringSamples()) {
        SCOPED_TRACE(sample++);
        UploadFileCheckpointV2 original;
        original.setBucket(value);
        original.setKey(value);
        original.setUploadId(value);
        original.setSseAlgorithm(value);
        original.setSseKeyMd5(value);
        original.setEncodingType(value);
        original.setFilePath(value);
        UploadFileInfoV2 info;
        info.setFileSize(7);
        info.setLastModified(123);
        original.setFileInfo(info);
        UploadFilePartInfoV2 part;
        part.setPartNum(1);
        part.setPartSize(7);
        part.setETag(value);
        part.setIsCompleted(true);
        original.setPartsInfo({part});
        ASSERT_NO_THROW(original.dump(path));
        EXPECT_NO_THROW(json::parse(readFile(path)));

        UploadFileCheckpointV2 loaded;
        ASSERT_NO_THROW(loaded.load(path));
        EXPECT_EQ(value, loaded.getBucket());
        EXPECT_EQ(value, loaded.getKey());
        EXPECT_EQ(value, loaded.getUploadId());
        EXPECT_EQ(value, loaded.getSseAlgorithm());
        EXPECT_EQ(value, loaded.getSseKeyMd5());
        EXPECT_EQ(value, loaded.getEncodingType());
        EXPECT_EQ(FileUtils::stringToUTF8(value), loaded.getFilePath());
        ASSERT_EQ(1u, loaded.getPartsInfo().size());
        EXPECT_EQ(value, loaded.getPartsInfo()[0].getETag());
        EXPECT_TRUE(loaded.isValid(7, 123, value, value));
    }
}

TEST_F(CheckpointRegressionTest, DownloadStringsRoundTripWithoutByteLoss) {
    const auto path = file("download.checkpoint");
    size_t sample = 0;
    for (const auto& value : stringSamples()) {
        SCOPED_TRACE(sample++);
        DownloadFileCheckpoint original;
        original.setBucket(value);
        original.setKey(value);
        original.setVersionId(value);
        original.setIfMatch(value);
        original.setIfNoneMatch(value);
        original.setIfModifiedSince(value);
        original.setIfUnmodifiedSince(value);
        original.setSseAlgorithm(value);
        original.setSseKeyMd5(value);
        DownloadFileObjectInfo info;
        info.setETag(value);
        info.setLastModified(value);
        original.setObjectInfo(info);
        DownloadFileFileInfo fileInfo;
        fileInfo.setFilePath(value);
        fileInfo.setTempFilePath(value);
        original.setFileInfo(fileInfo);
        ASSERT_NO_THROW(original.dump(path));
        DownloadFileCheckpoint loaded;
        ASSERT_NO_THROW(loaded.load(path));
        EXPECT_EQ(value, loaded.getBucket());
        EXPECT_EQ(value, loaded.getKey());
        EXPECT_EQ(value, loaded.getVersionId());
        EXPECT_EQ(value, loaded.getIfMatch());
        EXPECT_EQ(value, loaded.getIfNoneMatch());
        EXPECT_EQ(value, loaded.getIfModifiedSince());
        EXPECT_EQ(value, loaded.getIfUnmodifiedSince());
        EXPECT_EQ(value, loaded.getSseAlgorithm());
        EXPECT_EQ(value, loaded.getSseKeyMd5());
        EXPECT_EQ(value, loaded.getObjectInfo().getETag());
        EXPECT_EQ(value, loaded.getObjectInfo().getLastModified());
        EXPECT_EQ(FileUtils::stringToUTF8(value), loaded.getFileInfo().getFilePath());
        EXPECT_EQ(FileUtils::stringToUTF8(value), loaded.getFileInfo().getTempFilePath());
    }
}

TEST_F(CheckpointRegressionTest, CopyStringsRoundTripWithoutByteLoss) {
    const auto path = file("copy.checkpoint");
    size_t sample = 0;
    for (const auto& value : stringSamples()) {
        SCOPED_TRACE(sample++);
        ResumableCopyCheckpoint original;
        original.setBucket(value);
        original.setKey(value);
        original.setUploadId(value);
        original.setCopySourceIfMatch(value);
        original.setCopySourceIfNoneMatch(value);
        original.setCopySourceIfModifiedSince(value);
        original.setCopySourceIfUnmodifiedSince(value);
        original.setCopySourceSsecAlgorithm(value);
        original.setCopySourceSsecKeyMd5(value);
        original.setSsecAlgorithm(value);
        original.setSsecKeyMd5(value);
        original.setEncodingType(value);
        ResumableCopyCopySourceObjectInfo info;
        info.setETag(value);
        info.setLastModified(value);
        original.setCopySourceObjectInfo(info);
        ResumableCopyPartInfo part;
        part.setETag(value);
        part.setPartNum(1);
        part.setIsCompleted(true);
        original.setPartsInfo({part});
        ASSERT_NO_THROW(original.dump(path));
        ResumableCopyCheckpoint loaded;
        ASSERT_NO_THROW(loaded.load(path));
        EXPECT_EQ(value, loaded.getBucket());
        EXPECT_EQ(value, loaded.getKey());
        EXPECT_EQ(value, loaded.getUploadId());
        EXPECT_EQ(value, loaded.getCopySourceIfMatch());
        EXPECT_EQ(value, loaded.getCopySourceIfNoneMatch());
        EXPECT_EQ(value, loaded.getCopySourceIfModifiedSince());
        EXPECT_EQ(value, loaded.getCopySourceIfUnmodifiedSince());
        EXPECT_EQ(value, loaded.getCopySourceSsecAlgorithm());
        EXPECT_EQ(value, loaded.getCopySourceSsecKeyMd5());
        EXPECT_EQ(value, loaded.getSsecAlgorithm());
        EXPECT_EQ(value, loaded.getSsecKeyMd5());
        EXPECT_EQ(value, loaded.getEncodingType());
        EXPECT_EQ(value, loaded.getCopySourceObjectInfo().getETag());
        EXPECT_EQ(value, loaded.getCopySourceObjectInfo().getLastModified());
        ASSERT_EQ(1u, loaded.getPartsInfo().size());
        EXPECT_EQ(value, loaded.getPartsInfo()[0].getETag());
    }
}

TEST_F(CheckpointRegressionTest, ValidUtf8RetainsLegacyJsonRepresentation) {
    const std::string value = "\xe4\xb8\xad\xe6\x96\x87-\xf0\x9f\x98\x80";
    UploadFileCheckpointV2 checkpoint;
    checkpoint.setBucket("bucket");
    checkpoint.setKey(value);
    UploadFilePartInfoV2 part;
    part.setETag(value);
    checkpoint.setPartsInfo({part});
    const auto path = file("legacy-format.checkpoint");
    checkpoint.dump(path);
    const auto document = json::parse(readFile(path));
    EXPECT_TRUE(document.at("Key").is_string());
    EXPECT_EQ(value, document.at("Key").get<std::string>());
    ASSERT_TRUE(document.at("PartsInfo").at(0).is_string());
    const auto nested = json::parse(document.at("PartsInfo").at(0).get<std::string>());
    EXPECT_TRUE(nested.at("ETag").is_string());
    EXPECT_EQ(value, nested.at("ETag").get<std::string>());
    EXPECT_EQ(std::string::npos, readFile(path).find("__tos_checkpoint_bytes_v1"));
}

TEST_F(CheckpointRegressionTest, StandalonePartETagRoundTripsAllByteValues) {
    size_t sample = 0;
    for (const auto& value : stringSamples()) {
        SCOPED_TRACE(sample++);
        UploadFilePartInfoV2 original;
        original.setETag(value);
        UploadFilePartInfoV2 loaded;
        ASSERT_NO_THROW(loaded.load(original.dump()));
        EXPECT_EQ(value, loaded.getETag());
    }
}

TEST_F(CheckpointRegressionTest, InvalidUtf8UsesAnUnambiguousByteEnvelope) {
    UploadFilePartInfoV2 part;
    const std::string value = std::string("etag-\0", 6) + "\xc1\xff";
    part.setETag(value);
    const auto encoded = json::parse(part.dump());
    ASSERT_TRUE(encoded.at("ETag").is_object());
    const auto& bytes = encoded.at("ETag").at(CheckpointJson::bytesKey());
    ASSERT_TRUE(bytes.is_array());
    ASSERT_EQ(value.size(), bytes.size());
    for (size_t index = 0; index < value.size(); ++index) {
        EXPECT_EQ(static_cast<unsigned char>(value[index]), bytes.at(index).get<unsigned int>());
    }
    UploadFilePartInfoV2 loaded;
    loaded.load(part.dump());
    EXPECT_EQ(value, loaded.getETag());
}

TEST_F(CheckpointRegressionTest, MalformedByteEnvelopesAreRejectedWithoutThrowingFromCheckpointLoad) {
    const char* key = CheckpointJson::bytesKey();
    const std::vector<json> malformed = {json{{key, "not-an-array"}},
                                         json{{key, json::array({-1})}},
                                         json{{key, json::array({256})}},
                                         json{{key, json::array({1.5})}},
                                         json{{key, json::array({true})}},
                                         json{{key, json::array({(std::numeric_limits<uint64_t>::max)()})}},
                                         json{{key, json::array({65})}, {"extra", 1}},
                                         json{{"__tos_checkpoint_bytes_v2", json::array({65})}}};
    const auto path = file("malformed.checkpoint");
    for (size_t index = 0; index < malformed.size(); ++index) {
        SCOPED_TRACE(index);
        const json document = {{"Bucket", "bucket"}, {"Key", malformed[index]}};
        for (int operation = 0; operation < 3; ++operation) {
            writeFile(path, document.dump());
            if (operation == 0) {
                UploadFileCheckpointV2 loaded;
                EXPECT_NO_THROW(loaded.load(path));
                EXPECT_TRUE(loaded.getBucket().empty());
            } else if (operation == 1) {
                DownloadFileCheckpoint loaded;
                EXPECT_NO_THROW(loaded.load(path));
                EXPECT_TRUE(loaded.getBucket().empty());
            } else {
                ResumableCopyCheckpoint loaded;
                EXPECT_NO_THROW(loaded.load(path));
                EXPECT_TRUE(loaded.getBucket().empty());
            }
            EXPECT_FALSE(std::ifstream(path).good());
        }
    }
}

TEST_F(CheckpointRegressionTest, RandomByteStringsAndNestedValuesRoundTrip) {
    std::mt19937 random(20261105);
    for (size_t sample = 0; sample < 256; ++sample) {
        SCOPED_TRACE(sample);
        std::string bytes;
        for (size_t index = 0, length = random() % 256; index < length; ++index) {
            bytes.push_back(static_cast<char>(random() & 255));
        }
        const json original = {{"field", bytes}, {"array", json::array({bytes, "valid-utf8"})}};
        const auto encoded = CheckpointJson::dump(original);
        EXPECT_EQ(original, CheckpointJson::parse(encoded));
        EXPECT_EQ(encoded, CheckpointJson::dump(CheckpointJson::parse(encoded)));
    }
}

TEST_F(CheckpointRegressionTest, ExcessiveJsonNestingIsRejectedWithoutStackOverflow) {
    const std::string document =
            "{\"Bucket\":\"bucket\",\"Unexpected\":" + std::string(256, '[') + "0" + std::string(256, ']') + "}";
    const auto path = file("nested.checkpoint");
    writeFile(path, document);
    UploadFileCheckpointV2 checkpoint;
    EXPECT_NO_THROW(checkpoint.load(path));
    EXPECT_TRUE(checkpoint.getBucket().empty());
    EXPECT_FALSE(std::ifstream(path).good());
}

TEST_F(CheckpointRegressionTest, OldDownloadAndCopyFieldNamesRemainReadable) {
    const auto path = file("old.checkpoint");
    for (const auto& algorithm : {"SSEAlgorithm", "SSECustomerAlgorithm"}) {
        json document = {{algorithm, "AES256"}};
        writeFile(path, document.dump());
        DownloadFileCheckpoint loaded;
        loaded.load(path);
        EXPECT_EQ("AES256", loaded.getSseAlgorithm());
    }
    for (bool oldWriter : {false, true}) {
        json document = {{oldWriter ? "IfMatch" : "CopySourceIfMatch", "match"},
                         {oldWriter ? "IfNoneMatch" : "CopySourceIfNoneMatch", "none"},
                         {oldWriter ? "IfModifiedSince" : "CopySourceIfModifiedSince", "modified"},
                         {oldWriter ? "IfUnmodifiedSince" : "CopySourceIfUnmodifiedSince", "unmodified"},
                         {oldWriter ? "SSECustomerAlgorithm" : "SSECAlgorithm", "AES256"},
                         {oldWriter ? "SSECustomerMD5" : "SSECKeyMD5", "md5"}};
        writeFile(path, document.dump());
        ResumableCopyCheckpoint loaded;
        loaded.load(path);
        EXPECT_EQ("match", loaded.getCopySourceIfMatch());
        EXPECT_EQ("none", loaded.getCopySourceIfNoneMatch());
        EXPECT_EQ("modified", loaded.getCopySourceIfModifiedSince());
        EXPECT_EQ("unmodified", loaded.getCopySourceIfUnmodifiedSince());
        EXPECT_EQ("AES256", loaded.getSsecAlgorithm());
        EXPECT_EQ("md5", loaded.getSsecKeyMd5());
    }
}

TEST_F(CheckpointRegressionTest, LegacyReleaseCheckpointsRoundTripByteForByte) {
    // Golden records emitted identically by 2.6.19 (9efa0d4) and 2.6.27 (d17068f).
    const std::vector<std::string> legacy = {
            R"({"Bucket":"bucket","EncodingType":"","FileInfo":"{\"FileSize\":10,\"LastModified\":123}",)"
            R"("FilePath":"/tmp/legacy-ascii","Key":"legacy-ascii","PartSize":0,"PartsInfo":[)"
            R"("{\"ETag\":\"legacy-ascii\",\"HashCrc64ecma\":0,\"IsCompleted\":true,\"Offset\":0,)"
            R"(\"PartNum\":1,\"PartSize\":10}"],"SSECustomerAlgorithm":"","SSECustomerMD5":"","UploadID":"upload-id"})",
            R"({"Bucket":"bucket","FileInfo":"{\"FilePath\":\"\",\"TempFilePath\":\"\"}","IfMatch":"",)"
            R"("IfModifiedSince":"","IfNoneMatch":"","IfUnmodifiedSince":"","Key":"legacy-ascii",)"
            R"("ObjectInfo":"{\"ETag\":\"legacy-ascii\",\"HashCrc64ecma\":0,\"LastModified\":\"\",\"ObjectSize\":0}",)"
            R"("PartSize":0,"PartsInfo":[],"SSECustomerAlgorithm":"AES256","SSECustomerMD5":"","VersionID":""})",
            R"({"Bucket":"bucket","CopySourceObjectInfo":"{\"ETag\":\"\",\"HashCrc64ecma\":0,)"
            R"(\"LastModified\":\"\",\"ObjectSize\":0}","CopySourceSSECAlgorithm":"","CopySourceSSECKeyMD5":"",)"
            R"("EncodingType":"","IfMatch":"legacy-ascii","IfModifiedSince":"","IfNoneMatch":"",)"
            R"("IfUnmodifiedSince":"","Key":"legacy-ascii","PartSize":0,"PartsInfo":[)"
            R"("{\"CopySourceRangeEnd\":0,\"CopySourceRangeStart\":0,\"ETag\":\"legacy-ascii\",)"
            R"(\"IsCompleted\":true,\"PartNum\":1}"],"SSECustomerAlgorithm":"AES256",)"
            R"("SSECustomerMD5":"","UploadId":"copy-id"})"};
    const auto path = file("legacy-release.checkpoint");
    const auto rewritten = file("rewritten.checkpoint");
    for (size_t operation = 0; operation < legacy.size(); ++operation) {
        SCOPED_TRACE(operation);
        writeFile(path, legacy[operation]);
        if (operation == 0) {
            UploadFileCheckpointV2 loaded;
            ASSERT_NO_THROW(loaded.load(path));
            EXPECT_TRUE(loaded.isValid(10, 123, "bucket", "legacy-ascii"));
            EXPECT_EQ("upload-id", loaded.getUploadId());
            ASSERT_EQ(1u, loaded.getPartsInfo().size());
            EXPECT_TRUE(loaded.getPartsInfo()[0].isCompleted());
            EXPECT_EQ("legacy-ascii", loaded.getPartsInfo()[0].getETag());
            loaded.dump(rewritten);
        } else if (operation == 1) {
            DownloadFileCheckpoint loaded;
            ASSERT_NO_THROW(loaded.load(path));
            EXPECT_EQ("legacy-ascii", loaded.getKey());
            EXPECT_EQ("legacy-ascii", loaded.getObjectInfo().getETag());
            EXPECT_EQ("AES256", loaded.getSseAlgorithm());
            loaded.dump(rewritten);
        } else {
            ResumableCopyCheckpoint loaded;
            ASSERT_NO_THROW(loaded.load(path));
            EXPECT_EQ("copy-id", loaded.getUploadId());
            EXPECT_EQ("legacy-ascii", loaded.getCopySourceIfMatch());
            EXPECT_EQ("AES256", loaded.getSsecAlgorithm());
            ASSERT_EQ(1u, loaded.getPartsInfo().size());
            EXPECT_TRUE(loaded.getPartsInfo()[0].isCompleted());
            EXPECT_EQ("legacy-ascii", loaded.getPartsInfo()[0].getETag());
            loaded.dump(rewritten);
        }
        EXPECT_EQ(legacy[operation], readFile(path));
        EXPECT_EQ(legacy[operation], readFile(rewritten));
    }
}

TEST_F(CheckpointRegressionTest, ExistingReaderFieldNamesTakePrecedenceOverLegacyAliases) {
    const auto path = file("conflicting-aliases.checkpoint");
    writeFile(path, json{{"SSEAlgorithm", "preferred"}, {"SSECustomerAlgorithm", "legacy"}}.dump());
    DownloadFileCheckpoint download;
    download.load(path);
    EXPECT_EQ("preferred", download.getSseAlgorithm());
    const json document = {{"CopySourceIfMatch", "preferred-match"},
                           {"IfMatch", "legacy-match"},
                           {"CopySourceIfNoneMatch", "preferred-none"},
                           {"IfNoneMatch", "legacy-none"},
                           {"CopySourceIfModifiedSince", "preferred-modified"},
                           {"IfModifiedSince", "legacy-modified"},
                           {"CopySourceIfUnmodifiedSince", "preferred-unmodified"},
                           {"IfUnmodifiedSince", "legacy-unmodified"},
                           {"SSECAlgorithm", "preferred-algorithm"},
                           {"SSECustomerAlgorithm", "legacy-algorithm"},
                           {"SSECKeyMD5", "preferred-md5"},
                           {"SSECustomerMD5", "legacy-md5"}};
    writeFile(path, document.dump());
    ResumableCopyCheckpoint copy;
    copy.load(path);
    EXPECT_EQ("preferred-match", copy.getCopySourceIfMatch());
    EXPECT_EQ("preferred-none", copy.getCopySourceIfNoneMatch());
    EXPECT_EQ("preferred-modified", copy.getCopySourceIfModifiedSince());
    EXPECT_EQ("preferred-unmodified", copy.getCopySourceIfUnmodifiedSince());
    EXPECT_EQ("preferred-algorithm", copy.getSsecAlgorithm());
    EXPECT_EQ("preferred-md5", copy.getSsecKeyMd5());
}

TEST_F(CheckpointRegressionTest, MaximumUnsignedCrcAndLargeOffsetsRemainExact) {
    UploadFilePartInfoV2 part;
    part.setETag("\xe4\xb8\xad\xe6\x96\x87");
    part.setHashCrc64Result((std::numeric_limits<uint64_t>::max)());
    part.setOffset((std::numeric_limits<int64_t>::max)() - 10);
    part.setPartSize(10);
    part.setPartNum(10000);
    part.setIsCompleted(true);
    const auto serialized = part.dump();
    UploadFilePartInfoV2 loaded;
    loaded.load(serialized);
    EXPECT_EQ(part.getHashCrc64Result(), loaded.getHashCrc64Result());
    EXPECT_EQ(part.getOffset(), loaded.getOffset());
    EXPECT_EQ(part.getPartSize(), loaded.getPartSize());
    EXPECT_EQ(part.getPartNum(), loaded.getPartNum());
    EXPECT_EQ(serialized, loaded.dump());
}

TEST_F(CheckpointRegressionTest, MissingExplicitFilenameDoesNotNeedPrecreation) {
    for (int operation = 0; operation < 3; ++operation) {
        const auto path = file("missing-" + std::to_string(operation) + ".checkpoint");
        EXPECT_EQ(path, resolve(operation, path));
        EXPECT_FALSE(std::ifstream(path).good());
    }
}

TEST_F(CheckpointRegressionTest, MissingFilenameCreatesOnlyItsParentDirectory) {
    for (int operation = 0; operation < 3; ++operation) {
        const auto parent = root + TOS_PATH_DELIMITER + "parent-" + std::to_string(operation);
        directories.push_back(parent);
        const auto path = parent + TOS_PATH_DELIMITER + "missing.checkpoint";
        files.push_back(path);
        EXPECT_EQ(path, resolve(operation, path));
        EXPECT_FALSE(std::ifstream(path).good());
        writeFile(path, "checkpoint");
        EXPECT_EQ("checkpoint", readFile(path));
    }
}

TEST_F(CheckpointRegressionTest, ExistingFileIsNotTruncatedByPathResolution) {
    const auto path = file("existing.checkpoint");
    writeFile(path, "existing-state");
    for (int operation = 0; operation < 3; ++operation) {
        EXPECT_EQ(path, resolve(operation, path));
        EXPECT_EQ("existing-state", readFile(path));
    }
}

TEST_F(CheckpointRegressionTest, ExistingAndNewDirectoryPathsUseGeneratedNames) {
    const std::vector<std::string> extensions = {".upload", ".download", ".copy"};
    for (int operation = 0; operation < 3; ++operation) {
        for (bool exists : {false, true}) {
            const auto directory =
                    root + TOS_PATH_DELIMITER + "dir-" + std::to_string(operation) + (exists ? "-existing" : "-new");
            directories.push_back(directory);
            if (exists) {
                ASSERT_TRUE(FileUtils::CreateDir(directory, false));
            }
            const auto prefix = directory + TOS_PATH_DELIMITER;
            const auto resolved = resolve(operation, exists ? directory : prefix);
            EXPECT_EQ(0u, resolved.find(prefix));
            ASSERT_GT(resolved.size(), prefix.size());
            EXPECT_EQ(extensions[operation], resolved.substr(resolved.size() - extensions[operation].size()));
            EXPECT_FALSE(std::ifstream(resolved).good());
        }
    }
}

TEST_F(CheckpointRegressionTest, TrailingDirectorySeparatorRetainsLegacyPathSpelling) {
    for (int operation = 0; operation < 3; ++operation) {
        const auto withoutTrailingSeparator = resolve(operation, root);
        const auto withTrailingSeparator = resolve(operation, root + TOS_PATH_DELIMITER);
        EXPECT_EQ(root + TOS_PATH_DELIMITER + TOS_PATH_DELIMITER + withoutTrailingSeparator.substr(root.size() + 1),
                  withTrailingSeparator);
    }
}

TEST_F(CheckpointRegressionTest, FileUsedAsParentIsRejected) {
    const auto parent = file("not-a-directory");
    writeFile(parent, "preserve-me");
    for (int operation = 0; operation < 3; ++operation) {
        EXPECT_TRUE(resolve(operation, parent + TOS_PATH_DELIMITER + "child.checkpoint").empty());
        EXPECT_EQ("preserve-me", readFile(parent));
    }
}

}  // namespace
