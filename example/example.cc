#include "TosClientV2.h"

using namespace VolcengineTos;

void creatBucket(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName) {
    CreateBucketV2Input input(bucketName);
    auto output = client->createBucket(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::cout << output.result().getLocation() << std::endl;
}
void headBucket(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName) {
    HeadBucketV2Input input(bucketName);
    auto output = client->headBucket(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::cout << "The bucket region is: " << output.result().getRegion() << std::endl;
    std::cout << "The request id is: " << output.result().getRequestInfo().getRequestId() << std::endl;
}

void listBucket(const std::shared_ptr<TosClientV2>& client) {
    ListBucketsInput input;
    auto output = client->listBuckets(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    auto bkts = output.result().getBuckets();
    for (auto& bkt : bkts) {
        std::cout << bkt.getName() << std::endl;
    }
}

void deleteBucket(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName) {
    // 要求该 Bucket 已成功创建，否则会报错
    DeleteBucketInput input(bucketName);
    auto output = client->deleteBucket(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::cout << "Delete bucket success, the request id is: " << output.result().getRequestInfo().getRequestId()
              << std::endl;
}

void putObject(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
               const std::string& objectkey) {
    // 需要上传的对象数据，以 stringstream 的形式上传
    std::string data(
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'");
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input(bucketName, objectkey, ss);
    // 如果需要设置对象元数据，可按如下设置
    //    auto basicInput = input.getPutObjectBasicInput();
    //    basicInput.setContentLength(data.length());
    //    basicInput.setContentType("application/json");
    //    basicInput.setMeta({{"self-test", "yes"}});
    //    input.setPutObjectBasicInput(basicInput);
    auto output = client->putObject(input);
    if (!output.isSuccess()) {
        std::cout << "put object error: " << output.error().String() << std::endl;
        return;
    }
    std::cout << "put object success, object etag is: " << output.result().getETag() << std::endl;
}

static void ProgressCallback(std::shared_ptr<DataTransferStatus> datatransferstatus) {
    int64_t consumedBytes = datatransferstatus->consumedBytes_;
    int64_t totalBytes = datatransferstatus->totalBytes_;
    int64_t rwOnceBytes = datatransferstatus->rwOnceBytes_;
    DataTransferType type = datatransferstatus->type_;
    int64_t rate = 100 * consumedBytes / totalBytes;
    std::cout << "rate:" << rate
              << ","
                 "ConsumedBytes:"
              << consumedBytes << ","
              << "totalBytes:" << totalBytes << ","
              << "rwOnceBytes:" << rwOnceBytes << ","
              << "DataTransferType:" << type << std::endl;
}

void putObjectWithProcess(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                          const std::string& objectkey) {
    // 需要上传的对象数据，以 stringstream 的形式上传
    std::string data(
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'");
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input(bucketName, objectkey, ss);
    auto basicInput = input.getPutObjectBasicInput();
    DataTransferListener datatransferlistener = {ProgressCallback};
    basicInput.setDataTransferListener(datatransferlistener);
    input.setPutObjectBasicInput(basicInput);
    auto output = client->putObject(input);
    if (!output.isSuccess()) {
        std::cout << "put object error: " << output.error().String() << std::endl;
        return;
    }
    std::cout << "put object success, object etag is: " << output.result().getETag() << std::endl;
}

void putObjectWithRateLimiter(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                              const std::string& objectkey) {
    // 需要上传的对象数据，以 stringstream 的形式上传
    std::string data(
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'");
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input(bucketName, objectkey, ss);
    auto basicInput = input.getPutObjectBasicInput();
    std::shared_ptr<RateLimiter> RateLimiter(NewRateLimiter(1024 * 1024, 1024 * 1024));
    basicInput.setRateLimiter(RateLimiter);
    input.setPutObjectBasicInput(basicInput);
    auto output = client->putObject(input);
    if (!output.isSuccess()) {
        std::cout << "put object error: " << output.error().String() << std::endl;
        return;
    }
    std::cout << "put object success, object etag is: " << output.result().getETag() << std::endl;
}

void putObjectFromFile(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                       const std::string& objectkey, const std::string& fileName) {
    PutObjectFromFileInput input(bucketName, objectkey, fileName);
    auto output = client->putObjectFromFile(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::cout << "the object etag is: " << output.result().getPutObjectV2Output().getETag() << std::endl;
}

void getObject(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
               const std::string& objectkey) {
    GetObjectV2Input input(bucketName, objectkey);
    auto output = client->getObject(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::cout << output.result().getGetObjectBasicOutput().getETags() << std::endl;
}

void getObjectToFile(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                     const std::string& objectkey, const std::string filePath) {
    GetObjectToFileInput input(bucketName, objectkey, filePath);
    auto output = client->getObjectToFile(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
}

void headObject(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                const std::string& objectKey) {
    HeadObjectV2Input input(bucketName, objectKey);
    auto output = client->headObject(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    auto iter = output.result().getMeta().begin();
    while (iter != output.result().getMeta().end()) {
        std::cout << iter->first << "\t" << iter->second << std::endl;
        iter++;
    }
    std::cout << output.result().getContentType() << std::endl;
}

void deleteObject(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                  const std::string& objectKey) {
    DeleteObjectInput input(bucketName, objectKey);
    //    input.setVersionID("your object version id");
    auto output = client->deleteObject(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::cout << output.result().getRequestInfo().getRequestId() << std::endl;
}

void appendObject(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                  const std::string& objectKey) {
    auto part0 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (128 << 10); ++i) {
        *part0 << "1";
    }
    AppendObjectV2Input input(bucketName, objectKey, part0, 0);
    auto output = client->appendObject(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output.result().getNextAppendOffset() << std::endl;
    auto part1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (256 << 10); ++i) {
        *part1 << "1";
    }
    input.setContent(part1);
    input.setOffset(output.result().getNextAppendOffset());
    input.setPreHashCrc64Ecma(output.result().getHashCrc64ecma());
    auto output1 = client->appendObject(input);
    if (!output1.isSuccess()) {
        std::cout << output1.error().String() << std::endl;
    }
    std::cout << "Next Append Offset is: " << output1.result().getNextAppendOffset() << std::endl;
}

void putObjectAcl(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                  const std::string& objectKey) {
    PutObjectAclV2Input input;
    input.setAcl(ACLType::PublicReadWrite);
    input.setBucket(bucketName);
    input.setKey(objectKey);

    auto putRes = client->putObjectAcl(input);
    if (!putRes.isSuccess()) {
        std::cout << putRes.error().String() << std::endl;
        return;
    }
    std::cout << putRes.result().getRequestInfo().getRequestId() << std::endl;
}

void putObjectAcl2(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                   const std::string& objectKey) {
    PutObjectAclV2Input input(bucketName, objectKey);
    Owner owner;
    owner.setId("test-cid");
    GranteeV2 granteev2;
    granteev2.setType(GranteeType::CanonicalUser);
    std::string id = "test-cid";
    granteev2.setId(id);
    GrantV2 grantv2;
    grantv2.setGrantee(granteev2);
    grantv2.setPermission(PermissionType::FullControl);
    //    input.setAcl(ACLType::PublicReadWrite);
    input.setOwner(owner);
    input.setGrants({grantv2});
    auto putRes = client->putObjectAcl(input);
    if (!putRes.isSuccess()) {
        std::cout << putRes.error().String() << std::endl;
        return;
    }
    std::cout << putRes.result().getRequestInfo().getRequestId() << std::endl;
}

void getObjectAcl(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                  const std::string& objectKey) {
    GetObjectAclV2Input input(bucketName, objectKey);
    auto gotAcl = client->getObjectAcl(input);
    if (!gotAcl.isSuccess()) {
        std::cout << gotAcl.error().String() << std::endl;
    }
    PermissionType permission = gotAcl.result().getGrant()[0].getPermission();
    std::cout << PermissionTypetoString[permission] << std::endl;
}

void setObjectMeta(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                   const std::string& objectKey) {
    SetObjectMetaInput input(bucketName, objectKey);
    input.setContentType("application/json");
    input.setCacheControl("no-store");
    input.setContentDisposition("222.txt");
    std::map<std::string, std::string> meta{{"self-test", "no"}};
    input.setMeta(meta);
    auto output = client->setObjectMeta(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
}

void listObjects(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName) {
    int maxKeys = 100;
    ListObjectsV2Input input(bucketName);
    input.setMaxKeys(maxKeys);
    auto output = client->listObjects(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    for (const auto& i : output.result().getContents()) {
        std::cout << i.getKey() << std::endl;
    }
}

void listObjectsWithPrefix(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName) {
    int maxKeys = 100;
    std::string prefix("yourkeyPrefix");
    std::string marker("prefix/xxx");
    ListObjectsV2Input input(bucketName);
    input.setMaxKeys(maxKeys);
    input.setPrefix(prefix);
    input.setDelimiter("/");
    input.setMarker(marker);
    auto output = client->listObjects(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    for (const auto& i : output.result().getContents()) {
        std::cout << i.getKey() << std::endl;
    }
}

void listObjectVersion(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName) {
    ListObjectVersionsV2Input input(bucketName);
    input.setMaxKeys(100);
    auto output = client->listObjectVersions(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    for (const auto& i : output.result().getVersions()) {
        std::cout << "listObjectVersion:" << i.getKey() << "\t" << i.getVersionId() << std::endl;
    }
}

// void deleteObjectVersion(const std::shared_ptr<TosClientV2>& client, const std::string& bucket) {
//     std::string key("test0325-version");
//     client->putObject(bucket, key, std::make_shared<std::stringstream>("put your data"));
//     client->putObject(bucket, key, std::make_shared<std::stringstream>("put your new data"));
//     ListObjectVersionsInput input;
//     input.setMaxKeys(100);
//     input.setPrefix("test0325");
//     auto output = client->listObjectVersions(bucket, input);
//     for (const auto& i : output.result().getVersions()) {
//         std::cout << i.getKey() << "\t" << i.getVersionId() << std::endl;
//         RequestOptionBuilder rob;
//         rob.withVersionID(i.getVersionId());
//         auto res = client->deleteObject(bucket, key, rob);
//         std::cout << res.result().getVersionId() << std::endl;
//     }
// }

void getObjectRange(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                    const std::string& objectKey) {
    GetObjectV2Input input(bucketName, objectKey);
    input.setRangeStart(0);
    input.setRangeEnd(7);
    auto output = client->getObject(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
    std::cout << output.result().getContent()->rdbuf() << std::endl;
}

void copyObject(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName, const std::string& objectKey,
                const std::string& srcBucketName, const std::string& srcObjectKey) {
    CopyObjectV2Input input(bucketName, objectKey, srcBucketName, srcObjectKey);
    auto output = client->copyObject(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
    }
}

void uploadPartCopy(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName) {
    std::string objectKey("object-upload-part-copy-" + std::to_string(std::time(nullptr)));
    std::string dstKey("objectUploadPartCopy.data");
    auto ss = std::make_shared<std::stringstream>();
    for (int i = 0; i < (11 << 20); ++i) {
        *ss << 1;
    }
    PutObjectV2Input putInput(bucketName, objectKey, ss);
    auto srcPut = client->putObject(putInput);

    CreateMultipartUploadInput input_part_create(bucketName, dstKey);
    auto upload = client->createMultipartUpload(input_part_create);

    // 获取要copy的对象大小
    HeadObjectV2Input headInput(bucketName, objectKey);
    auto headOutput = client->headObject(headInput);
    if (!headOutput.isSuccess()) {
        std::cout << "head object error: " << headOutput.error().String() << std::endl;
        return;
    }

    int64_t size = headOutput.result().getContentLength();

    int partSize = 5 * 1024 * 1024;  // 10MB
    int partCount = (int)(size / partSize);
    std::vector<UploadedPartV2> copyParts(partCount);
    int64_t copySourceRangeEnd_ = 0;
    for (int i = 0; i < partCount; ++i) {
        int64_t partLen = partSize;
        if (partCount == i + 1 && (size % partLen) > 0) {
            partLen += size % (int64_t)partSize;
        }
        int64_t copySourceRangeStart_ = copySourceRangeEnd_;
        copySourceRangeEnd_ = copySourceRangeStart_ + partLen;

        UploadPartCopyV2Input input(bucketName, dstKey, bucketName, objectKey, i + 1, upload.result().getUploadId());
        input.setCopySourceRangeStart(copySourceRangeStart_);
        input.setCopySourceRangeEnd(copySourceRangeEnd_);
        auto res = client->uploadPartCopy(input);
        UploadedPartV2 temp(res.result().getPartNumber(), res.result().getETag());
        copyParts[i] = temp;
        copySourceRangeEnd_ = copySourceRangeEnd_ + 1;
    }

    CompleteMultipartUploadV2Input input(bucketName, dstKey, upload.result().getUploadId(), copyParts);
    auto complete = client->completeMultipartUpload(input);
    if (!complete.isSuccess()) {
        std::cout << "uploadPartCopy error: " << complete.error().String() << std::endl;
        return;
    }
}

void uploadPart(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                const std::string& objectKey) {
    CreateMultipartUploadInput input(bucketName, objectKey);
    auto upload = client->createMultipartUpload(input);
    if (!upload.isSuccess()) {
        std::cout << "createMultipartUpload error: " << upload.error().String() << std::endl;
        return;
    }
    std::cout << "createMultipartUpload succeed, begin to upload part" << std::endl;
    // generate some data..
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    UploadPartV2Input input_upload_part(bucketName, objectKey, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    // 上传分片对象
    auto part1 = client->uploadPart(input_upload_part);
    if (!part1.isSuccess()) {
        std::cout << "uploadPart error: " << part1.error().String() << std::endl;
        return;
    }

    std::cout << "uploadPart 1 succeed, begin to upload part 2" << std::endl;
    // generate some data..
    auto ss2 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (4 << 20); ++i) {
        *ss2 << 2;
    }
    UploadPartV2Input input_upload_part_2(bucketName, objectKey, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
    // 上传分片对象
    auto part2 = client->uploadPart(input_upload_part_2);
    if (!part2.isSuccess()) {
        std::cout << "uploadPart error: " << part2.error().String() << std::endl;
        return;
    }

    std::cout << "uploadPart 2 succeed, begin to completeMultipartUpload" << std::endl;
    auto part_1 = UploadedPartV2(part1.result().getPartNumber(), part1.result().getETag());
    auto part_2 = UploadedPartV2(part2.result().getPartNumber(), part2.result().getETag());
    std::vector<UploadedPartV2> vec = {part_1, part_2};
    CompleteMultipartUploadV2Input input_upload_complete(bucketName, objectKey, upload.result().getUploadId(), vec);
    auto com = client->completeMultipartUpload(input_upload_complete);
    if (!com.isSuccess()) {
        std::cout << "completeMultipartUpload error: " << com.error().String() << std::endl;
        return;
    }
    std::cout << "completeMultipartUpload success: " << com.result().getRequestInfo().getRequestId() << std::endl;
}
void uploadPartFromFile(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                        const std::string& objectKey, const std::string& filePath) {
    CreateMultipartUploadInput input(bucketName, objectKey);
    auto upload = client->createMultipartUpload(input);
    if (!upload.isSuccess()) {
        std::cout << "createMultipartUpload error: " << upload.error().String() << std::endl;
        return;
    }
    std::cout << "createMultipartUpload succeed, begin to upload part" << std::endl;
    // 当前分段在文件中的起始位置
    uint64_t offset = 0;
    // 当前分段长度
    uint64_t partsize = 5 << 20;
    UploadPartFromFileInput input_upload_part(bucketName, objectKey, upload.result().getUploadId(), 1, filePath, offset,
                                              partsize);
    // 上传分片对象
    auto part1 = client->uploadPartFromFile(input_upload_part);
    if (!part1.isSuccess()) {
        std::cout << "uploadPart error: " << part1.error().String() << std::endl;
        return;
    }

    std::cout << "uploadPart 1 succeed, begin to upload part 2" << std::endl;
    // generate some data..
    offset += partsize;
    partsize = (4 << 20);
    UploadPartFromFileInput input_upload_part_2(bucketName, objectKey, upload.result().getUploadId(), 2, filePath,
                                                offset, partsize);
    // 上传分片对象
    auto part2 = client->uploadPartFromFile(input_upload_part_2);
    if (!part2.isSuccess()) {
        std::cout << "uploadPart error: " << part2.error().String() << std::endl;
        return;
    }

    std::cout << "uploadPart 2 succeed, begin to completeMultipartUpload" << std::endl;
    auto part1_basic = part1.result().getUploadPartV2Output();
    auto part2_basic = part2.result().getUploadPartV2Output();
    auto part_1 = UploadedPartV2(part1_basic.getPartNumber(), part1_basic.getETag());
    auto part_2 = UploadedPartV2(part2_basic.getPartNumber(), part2_basic.getETag());
    std::vector<UploadedPartV2> vec = {part_1, part_2};
    CompleteMultipartUploadV2Input input_upload_complete(bucketName, objectKey, upload.result().getUploadId(), vec);
    auto com = client->completeMultipartUpload(input_upload_complete);
    if (!com.isSuccess()) {
        std::cout << "completeMultipartUpload error: " << com.error().String() << std::endl;
        return;
    }
    std::cout << "completeMultipartUpload success: " << com.result().getRequestInfo().getRequestId() << std::endl;
}
void uploadPartAbort(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                     const std::string& objectKey) {
    CreateMultipartUploadInput input(bucketName, objectKey);
    auto upload = client->createMultipartUpload(input);
    if (!upload.isSuccess()) {
        std::cout << "createMultipartUpload error: " << upload.error().String() << std::endl;
        return;
    }

    AbortMultipartUploadInput input1(bucketName, objectKey, upload.result().getUploadId());
    auto abort = client->abortMultipartUpload(input1);
    if (!abort.isSuccess()) {
        std::cout << abort.error().String() << std::endl;
        return;
    }
    std::cout << abort.result().getRequestInfo().getRequestId() << std::endl;
}

void uploadFile(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName, const std::string& objectKey,
                const std::string& path) {
    UploadFileV2Input input;
    CreateMultipartUploadInput input1(bucketName, objectKey);
    input.setCreateMultipartUploadInput(input1);
    // 默认分片大小 20MB
    input.setPartSize(20 * 1024 * 1024);
    // 并发上传分片的线程数 1-1000
    input.setTaskNum(100);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(path);
    auto output = client->uploadFile(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::cout << "the request id is: " << output.result().getRequestInfo().getRequestId() << std::endl;
}

void listMultipart(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName) {
    ListMultipartUploadsV2Input input(bucketName);
    input.setMaxUploads(100);
    auto res = client->listMultipartUploads(input);
    if (!res.isSuccess()) {
        std::cout << res.error().String() << std::endl;
        return;
    }
    for (const auto& i : res.result().getUploads()) {
        std::cout << "multipart key is: " << i.getKey() << std::endl;
    }
}

void listParts(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
               const std::string& objectKey) {
    CreateMultipartUploadInput input(bucketName, objectKey);
    auto upload = client->createMultipartUpload(input);
    if (!upload.isSuccess()) {
        std::cout << "createMultipartUpload error: " << upload.error().String() << std::endl;
        return;
    }
    std::cout << "createMultipartUpload succeed, begin to upload part" << std::endl;
    // generate some data..
    auto ss1 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (5 << 20); ++i) {
        *ss1 << 1;
    }
    UploadPartV2Input uploadPartInput(bucketName, objectKey, upload.result().getUploadId(), ss1->tellg(), 1, ss1);
    // 上传分片对象
    auto part1 = client->uploadPart(uploadPartInput);
    if (!part1.isSuccess()) {
        std::cout << "uploadPart error: " << part1.error().String() << std::endl;
        return;
    }

    std::cout << "uploadPart 1 succeed, begin to upload part 2" << std::endl;
    // generate some data..
    auto ss2 = std::make_shared<std::stringstream>();
    for (int i = 0; i < (4 << 20); ++i) {
        *ss2 << 2;
    }
    UploadPartV2Input uploadPartInput2(bucketName, objectKey, upload.result().getUploadId(), ss2->tellg(), 2, ss2);
    // 上传分片对象
    auto part2 = client->uploadPart(uploadPartInput2);
    if (!part2.isSuccess()) {
        std::cout << "uploadPart error: " << part2.error().String() << std::endl;
        return;
    }

    ListPartsInput listInput(bucketName, objectKey, upload.result().getUploadId());
    auto res = client->listParts(listInput);
    if (!res.isSuccess()) {
        std::cout << res.error().String() << std::endl;
        return;
    }
    for (const auto& i : res.result().getParts()) {
        std::cout << "uploaded part number is: " << i.getPartNumber() << std::endl;
        std::cout << "uploaded part size is: " << i.getSize() << std::endl;
        std::cout << "uploaded part etag is: " << i.getETag() << std::endl;
    }
    AbortMultipartUploadInput abortInput(bucketName, objectKey, upload.result().getUploadId());
    auto abort = client->abortMultipartUpload(abortInput);
    if (!abort.isSuccess()) {
        std::cout << abort.error().String() << std::endl;
        return;
    }
    std::cout << abort.result().getRequestInfo().getRequestId() << std::endl;
}

void deleteMultiObjects(const std::shared_ptr<TosClientV2>& client, const std::string& bucket) {
    std::vector<ObjectTobeDeleted> otds(2);
    ObjectTobeDeleted otd1;
    otd1.setKey("中文测试1644921863583");
    otds[0] = otd1;
    ObjectTobeDeleted otd2;
    otd2.setKey("中文测试1644921884327");
    otds[1] = otd2;
    DeleteMultiObjectsInput input;
    input.setQuiet(true);
    input.setObjectTobeDeleteds(otds);
    auto output = client->deleteMultiObjects(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::cout << "deleteMultiObjects success, the req id is: " << output.result().getRequestInfo().getRequestId()
              << std::endl;
}

void preSignedUrl(const std::shared_ptr<TosClientV2>& client, const std::string& bucket) {
    std::string data =
            "1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'1234567890abcdefghijklmnopqrstuvwxyz~!@#$%^&*()_+<>?,./   :'";
    auto ss = std::make_shared<std::stringstream>(data);
    std::string key("object-put-1");
    PutObjectV2Input input(bucket, key, ss);
    auto output = client->putObject(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::chrono::duration<int, std::ratio<100>> hs(3);
    auto res = client->preSignedURL("GET", bucket, key, hs);
    if (!res.isSuccess()) {
        std::cout << "preSignedUrl error: " << output.error().String() << std::endl;
        return;
    }
    std::cout << "the preSigned url is: " << res.result() << std::endl;
}

void downloadFile(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName,
                  const std::string& objectKey, const std::string& path) {
    DownloadFileInput input;
    HeadObjectV2Input headInput(bucketName, objectKey);
    input.setHeadObjectV2Input(headInput);
    // 并发上传分片的线程数 1-1000
    input.setTaskNum(1);
    // 开启 checkpoint 会在本地生成断点续传记录文件
    input.setEnableCheckpoint(true);
    // 默认分片大小 20MB
    input.setPartSize(5 * 1024 * 1024);
    // 待上传文件的路径，不可为空，不可为文件夹，建议设置绝对路径
    input.setFilePath(path);
    auto output = client->downloadFile(input);
    if (!output.isSuccess()) {
        std::cout << output.error().String() << std::endl;
        return;
    }
    std::cout << "the requestId is: " << output.result().getHeadObjectV2Output().getRequestInfo().getRequestId()
              << std::endl;
}

void CleanBucket(const std::shared_ptr<TosClientV2>& client, const std::string& bucketName) {
    ListMultipartUploadsV2Input input_multipart_list(bucketName);
    auto output_multipart_list = client->listMultipartUploads(input_multipart_list);
    auto uploads = output_multipart_list.result().getUploads();
    AbortMultipartUploadInput input_multipart_abort;
    input_multipart_abort.setBucket(bucketName);
    for (const auto& content : uploads) {
        input_multipart_abort.setKey(content.getKey());
        input_multipart_abort.setUploadId(content.getUploadId());
        auto output_abort = client->abortMultipartUpload(input_multipart_abort);
    }
    // list and delete
    ListObjectsV2Input input_obj_list;
    input_obj_list.setBucket(bucketName);
    auto output_obj_list = client->listObjects(input_obj_list);
    auto content_ = output_obj_list.result().getContents();
    std::vector<ListedObjectV2> testObject;
    DeleteObjectInput input_obj_delete;
    input_obj_delete.setBucket(bucketName);
    for (const auto& obj_ : content_) {
        std::string obj_name = obj_.getKey();
        input_obj_delete.setKey(obj_name);
        auto output_obj_delete = client->deleteObject(input_obj_delete);
        if (!output_obj_delete.isSuccess()) {
            std::cout << output_obj_delete.error().String() << std::endl;
            return;
        }
    }

    DeleteBucketInput input_bkt_delete;
    input_bkt_delete.setBucket(bucketName);
    auto output_bkt_delete = client->deleteBucket(input_bkt_delete);
    if (!output_bkt_delete.isSuccess()) {
        std::cout << output_bkt_delete.error().String() << std::endl;
        return;
    }
}

int main() {
    // 设置初始化参数，对象名和桶名
    std::string endpoint("Your endpoint");
    std::string region("your region");
    std::string ak("Your Access Key");
    std::string sk("Your Secret Key");
    std::string bucket("your bucket");
    std::string key("your object key");
    std::string fileName("Your File Path");
    std::string logFilePath("Your Log File Path");

    // 日志支持
    //    LogUtils::SetLogger(logFilePath, "tos-cpp-sdk", LogLevel::LogDebug);

    ClientConfig conf;
    conf.endPoint = endpoint;
    conf.enableCRC = true;
    InitializeClient();
    TosClientV2 client(region, ak, sk, conf);
    auto cli = std::make_shared<TosClientV2>(client);
    // CleanBucket(cli, bucket);
    creatBucket(cli, bucket);

    preSignedUrl(cli, bucket);
    // 桶接口
    listBucket(cli);
    headBucket(cli, bucket);
    //    deleteBucket(cli, bucket);
    //    creatBucket(cli, bucket);
    // 上传对象

    putObject(cli, bucket, key);
    // putObjectFromFile(cli, bucket, "putObjectFromFile" + key, fileName);
    // uploadFile(cli, bucket, key,fileName);
    appendObject(cli, bucket, "appendObject" + key);
    uploadPart(cli, bucket, "uploadPart" + key);
    uploadPartAbort(cli, bucket, "uploadPartAbort" + key);
    listMultipart(cli, bucket);
    listParts(cli, bucket, "listParts" + key);
    // 下载对象
    getObject(cli, bucket, key);
    // getObjectToFile(cli,bucket,key,fileName + "getObjectToFile");
    getObjectRange(cli, bucket, key);
    // 进度条特性支持
    putObjectWithProcess(cli, bucket, "process" + key);
    // 客户端限速支持
    putObjectWithRateLimiter(cli, bucket, "RateLimit " + key);
    // 管理对象
    headObject(cli, bucket, key);
    putObjectAcl(cli, bucket, key);
    getObjectAcl(cli, bucket, key);
    setObjectMeta(cli, bucket, key);
    listObjects(cli, bucket);
    listObjectVersion(cli, bucket);
    creatBucket(cli, "tobucket-" + bucket);
    copyObject(cli, "tobucket-" + bucket, "toObject-" + key, bucket, key);
    // CleanBucket(cli, "tobucket-" + bucket);
    uploadPartCopy(cli, bucket);

    deleteObject(cli, bucket, key);

    CloseClient();
}
