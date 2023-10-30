#include <iostream>
#include "OthersObjectSample.h"
#include <fstream>

using namespace VolcengineTos;

int OthersObjectSample::PutObjectWithMetaData() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 需要上传的对象数据，以 stringstream 的形式上传
    std::string data("Object data to be uploaded");
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input(bucketName, objectName, ss);
    // 如果需要设置对象元数据，可按如下设置
    auto basicInput = input.getPutObjectBasicInput();
    // 设置 ACL 为 PublicRead
    basicInput.setAcl(ACLType::PublicRead);
    // 设置 StorageClass 为 IA
    basicInput.setStorageClass(StorageClassType::IA);
    // 设置对象元数据
    basicInput.setMeta({{"self-test", "yes"}});
    // 设置ContentType
    basicInput.setContentType("application/json");
    //
    input.setPutObjectBasicInput(basicInput);
    auto output = client.putObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutObjectWithMetaData failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "PutObjectWithMetaData success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
static int64_t getFileSize(const std::string& file) {
    std::fstream f(file, std::ios::in | std::ios::binary);
    f.seekg(0, f.end);
    int64_t size = f.tellg();
    f.close();
    return size;
}
int OthersObjectSample::MultipartUploadWithMetaData() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 初始化分片上传事件
    CreateMultipartUploadInput input(bucketName, objectName);
    // 可以指定 ACL，StorageClass，用户自定义元数据等
    input.setAcl(ACLType::PublicRead);
    input.setStorageClass(StorageClassType::IA);
    input.setMeta({{"self-test", "yes"}});
    auto upload = client.createMultipartUpload(input);
    if (!upload.isSuccess()) {
        // 异常处理
        std::cout << "createMultipartUpload failed." << upload.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "createMultipartUpload success." << std::endl;

    // 准备上传的文件
    std::string fileToUpload = "yourLocalFilename";
    int64_t partSize = 100 * 1024;
    std::vector<UploadedPartV2> partResList;
    auto fileSize = getFileSize(fileToUpload);
    int partCount = static_cast<int>(fileSize / partSize);
    // 计算分片个数
    if (fileSize % partSize != 0) {
        partCount++;
    }

    // 对每一个分片进行上传
    auto uploadId = upload.result().getUploadId();
    for (int i = 1; i <= partCount; i++) {
        auto offset = partSize * (i - 1);
        auto size = (partSize < fileSize - offset) ? partSize : (fileSize - offset);
        std::shared_ptr<std::iostream> content =
                std::make_shared<std::fstream>(fileToUpload, std::ios::in | std::ios::binary);
        content->seekg(offset, std::ios::beg);

        UploadPartV2Input uploadPartInput(bucketName, objectName, uploadId, size, i, content);
        auto uploadPartOutput = client.uploadPart(uploadPartInput);
        if (!uploadPartOutput.isSuccess()) {
            std::cout << "uploadPart failed." << upload.error().String() << std::endl;
        }

        UploadedPartV2 part(i, uploadPartOutput.result().getETag());
        partResList.push_back(part);
    }

    // 完成分片上传
    CompleteMultipartUploadV2Input completeMultipartUploadInput(bucketName, objectName, uploadId, partResList);
    auto com = client.completeMultipartUpload(completeMultipartUploadInput);
    if (com.isSuccess()) {
        // 异常处理
        std::cout << "CompleteMultipartUpload failed." << upload.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "CompleteMultipartUpload success." << com.result().getRequestInfo().getRequestId() << std::endl;
    CloseClient();
    return 0;
}

int OthersObjectSample::SetObjectMeta() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    SetObjectMetaInput input(bucketName, objectName);
    // 如果需要设置对象元数据，可按如下设置
    input.setMeta({{"self-test", "yes"}});
    auto output = client.setObjectMeta(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "SetObjectMeta failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "SetObjectMeta success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int OthersObjectSample::GetObjectMeta() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    HeadObjectV2Input input(bucketName, objectName);
    auto output = client.headObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "HeadObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "HeadObject success." << std::endl;
    std::cout << output.result().getStringFormatStorageClass() << "," << output.result().getContentDisposition() << ","
              << output.result().getContentType() << "," << output.result().getContentLength() << std::endl;
    for (const auto& meta : output.result().getMeta()) {
        std::cout << meta.first << "," << meta.second << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}
int OthersObjectSample::DoesObjectExist() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    HeadObjectV2Input input(bucketName, objectName);
    auto output = client.headObject(input);
    if (!output.isSuccess()) {
        std::cout << "Object not exist." << output.error().String() << std::endl;
    }

    std::cout << "Object exist." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int OthersObjectSample::PutObjectWithAcl() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 需要上传的对象数据，以 stringstream 的形式上传
    std::string data("Object data to be uploaded");
    auto ss = std::make_shared<std::stringstream>(data);
    PutObjectV2Input input(bucketName, objectName, ss);
    // 如果需要设置对象元数据，可按如下设置
    auto basicInput = input.getPutObjectBasicInput();
    // 设置 ACL 为 PublicRead
    basicInput.setAcl(ACLType::PublicRead);
    input.setPutObjectBasicInput(basicInput);
    auto output = client.putObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutObjectWithMetaData failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "PutObjectWithMetaData success. the object etag:" << output.result().getETag() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int OthersObjectSample::MultipartUploadWithAcl() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 初始化分片上传事件
    CreateMultipartUploadInput input(bucketName, objectName);
    // 可以指定 ACL，StorageClass，用户自定义元数据等
    input.setAcl(ACLType::PublicRead);
    auto upload = client.createMultipartUpload(input);
    if (!upload.isSuccess()) {
        // 异常处理
        std::cout << "createMultipartUpload failed." << upload.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "createMultipartUpload success." << std::endl;

    // 准备上传的文件
    std::string fileToUpload = "yourLocalFilename";
    int64_t partSize = 100 * 1024;
    std::vector<UploadedPartV2> partResList;
    auto fileSize = getFileSize(fileToUpload);
    int partCount = static_cast<int>(fileSize / partSize);
    // 计算分片个数
    if (fileSize % partSize != 0) {
        partCount++;
    }

    // 对每一个分片进行上传
    auto uploadId = upload.result().getUploadId();
    for (int i = 1; i <= partCount; i++) {
        auto offset = partSize * (i - 1);
        auto size = (partSize < fileSize - offset) ? partSize : (fileSize - offset);
        std::shared_ptr<std::iostream> content =
                std::make_shared<std::fstream>(fileToUpload, std::ios::in | std::ios::binary);
        content->seekg(offset, std::ios::beg);

        UploadPartV2Input uploadPartInput(bucketName, objectName, uploadId, size, i, content);
        auto uploadPartOutput = client.uploadPart(uploadPartInput);
        if (uploadPartOutput.isSuccess()) {
            UploadedPartV2 part(i, uploadPartOutput.result().getETag());
            partResList.push_back(part);
        } else {
            std::cout << "uploadPart failed." << upload.error().String() << std::endl;
        }
    }

    // 完成分片上传
    CompleteMultipartUploadV2Input completeMultipartUploadInput(bucketName, objectName, uploadId, partResList);
    auto com = client.completeMultipartUpload(completeMultipartUploadInput);
    if (!com.isSuccess()) {
        // 异常处理
        std::cout << "CompleteMultipartUpload failed." << upload.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "CompleteMultipartUpload success." << com.result().getRequestInfo().getRequestId() << std::endl;
    CloseClient();
    return 0;
}

int OthersObjectSample::PutObjectAcl() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    PutObjectAclV2Input input(bucketName, objectName);
    // 如果需要设置对象元数据，可按如下设置
    input.setAcl(ACLType::PublicRead);
    auto output = client.putObjectAcl(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutObjectAcl failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "PutObjectAcl success." << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

int OthersObjectSample::SetObjectACLWithGrant() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 设置桶的访问权限为公共读，默认为私有。
    PutObjectAclV2Input input(bucketName, objectName);
    // 设置用户ID为1000000001具备FULL_CONTROL控制权, 所有用户具备桶的READ权限
    input.setGrantFullControl("id=\"1000000001\"");
    input.setGrantRead("canned=\"AllUsers\"");
    auto output = client.putObjectAcl(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "SetObjectACL failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "SetObjectACL success." << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

int OthersObjectSample::SetObjectACLByDetailedConf() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 设置桶的访问权限为公共读，默认为私有。
    PutObjectAclV2Input input(bucketName, objectName);
    Owner owner;
    owner.setId("test-cid");
    GranteeV2 granteev2;
    granteev2.setType(GranteeType::Group);
    granteev2.setCanned(CannedType::AllUsers);
    GrantV2 grantv2;
    grantv2.setGrantee(granteev2);
    grantv2.setPermission(PermissionType::WriteAcp);
    input.setOwner(owner);
    input.setGrants({grantv2});
    auto output = client.putObjectAcl(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "SetObjectACL failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "SetObjectACL success." << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

int OthersObjectSample::GetObjectAcl() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    GetObjectAclV2Input input(bucketName, objectName);
    auto output = client.getObjectAcl(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetOBjectAcl failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "GetObjectAcl success." << std::endl;
    // 打印 ACL 信息
    auto grant = output.result().getGrant();
    for (auto& g : grant) {
        std::cout << " the premission is:" << g.getStringFormatPermission()
                  << " the grantee id is:" << g.getGrantee().getId()
                  << " the display name is:" << g.getGrantee().getDisplayName()
                  << " the grantee is:" << g.getGrantee().getStringFormatType()
                  << " the canned is:" << g.getGrantee().getStringFormatCanned() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}
// 删除单对象
int OthersObjectSample::DeleteObject() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    DeleteObjectInput input(bucketName, objectName);
    auto output = client.deleteObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "DeleteObject success." << std::endl;
    std::cout << " version id is:" << output.result().getVersionId()
              << " request id is:" << output.result().getRequestInfo().getRequestId()
              << " is delete marker:" << output.result().isDeleteMarker() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
// 删除带版本对象
int OthersObjectSample::DeleteObjectVersioning() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";
    // 填写对象的 versionId
    std::string versionId = "your object version";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    DeleteObjectInput input(bucketName, objectName);
    input.setVersionID(versionId);
    auto output = client.deleteObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "DeleteObject success." << std::endl;
    std::cout << " version id is:" << output.result().getVersionId()
              << " is delete marker:" << output.result().isDeleteMarker() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}
// 批量删除对象
int OthersObjectSample::BatchDeleteObject() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject1.txt和exampledir/exampleobject2.txt。
    std::string objectName1 = "exampledir/exampleobject1.txt";
    std::string objectName2 = "exampledir/exampleobject2.txt";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    DeleteMultiObjectsInput input;
    input.setBucket(bucketName);
    ObjectTobeDeleted object1(objectName1);
    ObjectTobeDeleted object2(objectName2);
    input.addObjectTobeDeleted(object1);
    input.addObjectTobeDeleted(object2);

    auto output = client.deleteMultiObjects(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteMultiObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "DeleteMultiObjects success." << std::endl;
    for (auto& deleted : output.result().getDeleteds()) {
        std::cout << " object key is:" << deleted.getKey() << " object version id is:" << deleted.getVersionId()
                  << " is delete marker:" << deleted.isDeleteMarker()
                  << " delete marker version id is:" << deleted.getDeleteMarkerVersionId() << std::endl;
    }
    for (auto& error : output.result().getErrors()) {
        std::cout << " delete err object key is:" << error.getKey()
                  << " delete err object version id is:" << error.getVersionId()
                  << " delete err message:" << error.getMessage() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}
// 批量删除带版本对象
int OthersObjectSample::BatchDeleteObjectVersioning() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject1.txt和exampledir/exampleobject2.txt。
    std::string objectName1 = "exampledir/exampleobject1.txt";
    std::string objectName2 = "exampledir/exampleobject2.txt";
    // 填写对象的 versionId
    std::string versionId1 = "your object1 version";
    std::string versionId2 = "your object2 version";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    DeleteMultiObjectsInput input;
    input.setBucket(bucketName);
    ObjectTobeDeleted object1(objectName1, versionId1);
    ObjectTobeDeleted object2(objectName2, versionId2);
    input.addObjectTobeDeleted(object1);
    input.addObjectTobeDeleted(object2);
    // 桶开启多版本并指定删除对象的版本号，则为直接标删除
    auto output = client.deleteMultiObjects(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteMultiObjects failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "DeleteMultiObjects success." << std::endl;
    for (auto& deleted : output.result().getDeleteds()) {
        std::cout << " object key is:" << deleted.getKey() << " object version id is:" << deleted.getVersionId()
                  << " is delete marker:" << deleted.isDeleteMarker()
                  << " delete marker version id is:" << deleted.getDeleteMarkerVersionId() << std::endl;
    }
    for (auto& error : output.result().getErrors()) {
        std::cout << " delete err object key is:" << error.getKey()
                  << " delete err object version id is:" << error.getVersionId()
                  << " delete err message:" << error.getMessage() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}
// 结合列举对象接口删除桶内对象
int OthersObjectSample::ListAndDeleteObjects() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectVersionsV2Input input(bucketName);
    bool isTruncated = true;
    std::string nextKeyMarker = "";

    DeleteObjectInput deleteObjectInput;
    deleteObjectInput.setBucket(bucketName);
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        auto output = client.listObjectVersions(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListObjects failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        nextKeyMarker = output.result().getNextKeyMarker();
        isTruncated = output.result().isTruncated();

        for (const auto& versions : output.result().getVersions()) {
            deleteObjectInput.setKey(versions.getKey());
            deleteObjectInput.setVersionID(versions.getVersionId());
            auto deleteOutput = client.deleteObject(deleteObjectInput);
            if (!deleteOutput.isSuccess()) {
                if (output.error().getStatusCode() != 404 && output.error().getCode() != "No Such Key") {
                    std::cout << "DeleteObject failed:" << output.error().String() << std::endl;
                }
            }
        }
        for (const auto& deleteMarker : output.result().getDeleteMarkers()) {
            deleteObjectInput.setKey(deleteMarker.getKey());
            deleteObjectInput.setVersionID(deleteMarker.getVersionId());
            auto deleteOutput = client.deleteObject(deleteObjectInput);
            if (!deleteOutput.isSuccess()) {
                if (output.error().getStatusCode() != 404) {
                    std::cout << "DeleteObject failed:" << output.error().String() << std::endl;
                }
            }
        }
    }

    // 列举已上传分片
    ListMultipartUploadsV2Input input2(bucketName);
    AbortMultipartUploadInput abortMultipartUploadInput;
    abortMultipartUploadInput.setBucket(bucketName);
    // isTruncated用于控制是否列举完全，nextKeyMarker用于递归列举
    isTruncated = true;
    nextKeyMarker = "";
    while (isTruncated) {
        input2.setKeyMarker(nextKeyMarker);
        auto output = client.listMultipartUploads(input2);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListMultipartUploads failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        nextKeyMarker = output.result().getNextKeyMarker();
        isTruncated = output.result().isTruncated();

        std::cout << "ListMultipartUploads success." << std::endl;
        for (const auto& part : output.result().getUploads()) {
            abortMultipartUploadInput.setKey(part.getKey());
            abortMultipartUploadInput.setUploadId(part.getUploadId());
            auto abortOutput = client.abortMultipartUpload(abortMultipartUploadInput);
            if (!abortOutput.isSuccess()) {
                std::cout << "DeletePart failed:" << abortOutput.error().String() << std::endl;
            }
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int OthersObjectSample::ListAndDeleteObjects2() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectVersionsV2Input input(bucketName);
    bool isTruncated = true;
    std::string nextKeyMarker = "";

    DeleteObjectInput deleteObjectInput;
    deleteObjectInput.setBucket(bucketName);
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        auto output = client.listObjectVersions(input);
        if (!output.isSuccess()) {
            nextKeyMarker = output.result().getNextKeyMarker();
            isTruncated = output.result().isTruncated();

            for (const auto& versions : output.result().getVersions()) {
                deleteObjectInput.setKey(versions.getKey());
                auto deleteOutput = client.deleteObject(deleteObjectInput);
                if (!deleteOutput.isSuccess()) {
                    if (output.error().getStatusCode() != 404) {
                        std::cout << "DeleteObject failed:" << output.error().String() << std::endl;
                    }
                }
            }
            for (const auto& deleteMarker : output.result().getDeleteMarkers()) {
                deleteObjectInput.setKey(deleteMarker.getKey());
                auto deleteOutput = client.deleteObject(deleteObjectInput);
                if (!deleteOutput.isSuccess()) {
                    std::cout << "DeleteObject failed:" << output.error().String() << std::endl;
                }
            }
        } else {
            // 异常处理
            std::cout << "ListObjects failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int OthersObjectSample::ListAndDeleteObjectsVersioning() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    ListObjectVersionsV2Input input(bucketName);
    bool isTruncated = true;
    std::string nextKeyMarker = "";

    DeleteObjectInput deleteObjectInput;
    deleteObjectInput.setBucket(bucketName);
    while (isTruncated) {
        input.setKeyMarker(nextKeyMarker);
        auto output = client.listObjectVersions(input);
        if (!output.isSuccess()) {
            // 异常处理
            std::cout << "ListObjects failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }

        nextKeyMarker = output.result().getNextKeyMarker();
        isTruncated = output.result().isTruncated();

        for (const auto& versions : output.result().getVersions()) {
            deleteObjectInput.setKey(versions.getKey());
            deleteObjectInput.setVersionID(versions.getVersionId());
            auto deleteOutput = client.deleteObject(deleteObjectInput);
            if (!deleteOutput.isSuccess()) {
                std::cout << "DeleteObject failed:" << output.error().String() << std::endl;
            }
        }
        for (const auto& deleteMarker : output.result().getDeleteMarkers()) {
            deleteObjectInput.setKey(deleteMarker.getKey());
            deleteObjectInput.setVersionID(deleteMarker.getVersionId());
            auto deleteOutput = client.deleteObject(deleteObjectInput);
            if (!deleteOutput.isSuccess()) {
                std::cout << "DeleteObject failed:" << output.error().String() << std::endl;
            }
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

// 管理对象标签
int OthersObjectSample::PutObjectTagging() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 若桶开启了多版本可通过 version_id = 'your object version id' 指定对应版本对象
    PutObjectTaggingInput input(bucketName, objectName);
    // 设置 tags
    Tag tag1("key1", "value1");
    Tag tag2("key2", "value2");
    TagSet tagSet({tag1, tag2});
    input.setTagSet(tagSet);
    auto output = client.putObjectTagging(input);
    if (!output.isSuccess()) {
        std::cout << "PutObjectTagging success." << std::endl;
    } else {
        // 异常处理
        std::cout << "PutObjectTagging failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int OthersObjectSample::GetObjectTagging() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 若桶开启了多版本可通过 version_id = 'your object version id' 指定对应版本对象
    GetObjectTaggingInput input(bucketName, objectName);
    auto output = client.getObjectTagging(input);
    if (!output.isSuccess()) {
        std::cout << "GetObjectTagging success." << std::endl;
        for (auto& tag : output.result().getTagSet().getTags()) {
            std::cout << " tag key is:" << tag.getKey() << " tag value is:" << tag.getValue() << std::endl;
        }
    } else {
        // 异常处理
        std::cout << "GetObjectTagging failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int OthersObjectSample::DeleteObjectTagging() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 若桶开启了多版本可通过 version_id = 'your object version id' 指定对应版本对象
    DeleteObjectTaggingInput input(bucketName, objectName);
    auto output = client.deleteObjectTagging(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteObjectTagging failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DeleteObjectTagging success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int OthersObjectSample::PutObjectWithErrorProcess() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    GetObjectV2Input input(bucketName, objectName);
    auto output = client.getObject(input);
    if (!output.isSuccess()) {
        std::cout << "GetObject success. the object etag:" << output.result().getGetObjectBasicOutput().getETags()
                  << std::endl;
    } else {
        auto error = output.error();
        if (error.isClientError()) {
            // 客户端错误
            std::cout << "TosClientError: {message=" << error.getMessage() << "}";
        } else {
            // 服务端错误
            std::cout << "TosServerError: {statusCode=" << error.getStatusCode() << ", "
                      << "code=" << error.getCode() << ", "
                      << "message=" << error.getMessage() << ", "
                      << "requestID=" << error.getRequestId() << ", "
                      << "hostID=" << error.getHostId() << "}";
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int OthersObjectSample::RestoreObject() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写Object完整路径，完整路径中不能包含Bucket名称，例如exampledir/exampleobject.txt。
    std::string objectName = "exampledir/exampleobject.txt";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    RestoreObjectInput input(bucketName, objectName, 10, RestoreJobParameters(TierType::TierExpedited));
    auto output = client.restoreObject(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "RestoreObject failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "RestoreObject success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
