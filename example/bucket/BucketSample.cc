#include "BucketSample.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace VolcengineTos;

int BucketSample::CreateBucket() {
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

    CreateBucketV2Input input(bucketName);

    // 如果创建桶的同时需要指定桶访问权限、数据容灾类型、存储类型，请参考如下代码：
    // 设置桶的访问权限为公共读，默认为私有。
    // input.setAcl(ACLType::PublicRead);
    // 设置桶的数据容灾为多 AZ 冗余存储，默认为单 AZ 冗余存储
    // input.setAzRedundancy(AzRedundancyType::MultiAz);
    // 设置桶的存储类型为标准存储。
    // input.setStorageClass(StorageClassType::STANDARD);

    // 创建桶
    auto output = client.createBucket(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "CreateBucket failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "CreateBucket Success, the bucket location:" << output.result().getLocation() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::ListBuckets() {
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

    // 列举桶
    ListBucketsInput input;
    auto output = client.listBuckets(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "ListBuckets failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    // 打印桶信息
    std::cout << "ListBuckets Success, the bucket count is :" << output.result().getBuckets().size() << std::endl;
    std::cout << "Bucket Info is" << std::endl;
    for (auto& result : output.result().getBuckets()) {
        std::cout << "bucketName: " << result.getName() << " location: " << result.getLocation()
                  << " createDate: " << result.getCreationDate() << std::endl;
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::GetBucketMetaDate() {
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

    // 获取桶元数据
    HeadBucketV2Input input(bucketName);
    auto output = client.headBucket(input);
    if (!output.isSuccess()) {
        // 桶不存在
        if (!output.error().isClientError() && output.error().getStatusCode() == 404) {
            std::cout << "Bucket not found." << std::endl;
        } else {
            std::cout << "HeadBucket failed." << output.error().String() << std::endl;
            // 释放网络等资源
            CloseClient();
            return -1;
        }
    }
    // 打印桶信息
    std::cout << "HeadBucket success, the bucket region is:" << output.result().getRegion()
              << " the storage class is:" << output.result().getStringFormatStorageClass()
              << " the azRedundancy is:" << output.result().getStringFormatAzRedundancy() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::DeleteBucket() {
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

    // 删除桶
    DeleteBucketInput input(bucketName);
    auto output = client.deleteBucket(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteBucket failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DeleteBucket success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::GetBucketLocation() {
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

    // 查看存储桶地域信息
    GetBucketLocationInput input(bucketName);
    auto output = client.getBucketLocation(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketLocation failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    // 打印存储桶地域、存储桶外网域名、存储桶内网域名
    std::cout << "GetBucketLocation success, "
              << "the bucket region:" << output.result().getRegion()
              << "the bucket ExtranetEndpoint:" << output.result().getExtranetEndpoint()
              << "the bucket IntranetEndpoint:" << output.result().getIntranetEndpoint() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::SetBucketACL() {
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

    // 设置桶的访问权限为公共读，默认为私有。
    PutBucketAclInput input(bucketName);
    input.setAcl(ACLType::PublicRead);
    auto output = client.putBucketAcl(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "SetBucketACL failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "SetBucketACL success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::SetBucketACLWithGrant() {
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

    // 设置桶的访问权限为公共读，默认为私有。
    PutBucketAclInput input(bucketName);
    // 设置用户ID为1000000001具备FULL_CONTROL控制权, 所有用户具备桶的READ权限
    input.setGrantFullControl("id=\"1000000001\"");
    input.setGrantRead("canned=\"AllUsers\"");
    auto output = client.putBucketAcl(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "SetBucketACL failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "SetBucketACL success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::SetBucketACLByDetailedConf() {
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

    // 设置桶的访问权限为公共读，默认为私有。
    PutBucketAclInput input(bucketName);
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
    auto output = client.putBucketAcl(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "SetBucketACL failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "SetBucketACL success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::GetBucketACL() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";

    PermissionType permission_;
    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 获取桶读写权限配置
    GetBucketAclInput input(bucketName);
    auto output = client.getBucketAcl(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketACL failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "GetBucketACL success." << std::endl;
    // 打印：
    // 授予者所拥有的 Bucket 权限，
    // 当 Type 为 CanonicalUser 时，表示权限授予者的 ID，
    // 权限被授予者的名称，
    // 权限授予者的类型，
    // 被授权的用户组。
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

int BucketSample::PutBucketPolicy() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 若要设置如下的 policy
    std::string Policy = R"({"Statement": [{"Sid": "internal public","Effect": "Allow","Action":
["tos:ListBucket"],"Principal": "*","Resource": ["trn:tos:::examplebucket/*","trn:tos:::examplebucket"]}]})";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    // 设置桶的策略配置
    PutBucketPolicyInput input(bucketName);
    input.setPolicy(Policy);
    auto output = client.putBucketPolicy(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutBucketPolicy failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutBucketPolicy success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::GetBucketPolicy() {
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

    // 获取桶的策略配置
    GetBucketPolicyInput input(bucketName);
    auto output = client.getBucketPolicy(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketPolicy failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    // 打印 Policy 信息
    std::cout << "GetBucketPolicy success, the bucket policy is:" << output.result().getPolicy() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::DeleteBucketPolicy() {
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

    // 删除桶的策略配置
    DeleteBucketPolicyInput input(bucketName);
    auto output = client.deleteBucketPolicy(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteBucketPolicy failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DeleteBucketPolicy success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::PutBucketStorageClass() {
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

    // 设置桶的存储类型为 IA
    PutBucketStorageClassInput input(bucketName, StorageClassType::IA);
    auto output = client.putBucketStorageClass(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutBucketStorageClass failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutBucketStorageClass success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::GetBucketStorageClass() {
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

    // 获取桶元数据
    HeadBucketV2Input input(bucketName);
    auto output = client.headBucket(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketStorageClass failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    // 打印桶的存储类型信息
    std::cout << "GetBucketStorageClass success, the bucket storage class is:"
              << output.result().getStringFormatStorageClass() << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::PutBucketLifecycle() {
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

    // 设置桶的生命周期
    PutBucketLifecycleInput input(bucketName);

    // 指定生命周期规则 rule1，该规则设置创建 60 天后过期
    auto rule1 = LifecycleRule();
    // 规则 ID
    rule1.setId("rule1");
    // 指定规则适用的前缀，匹配前缀
    rule1.setPrefix("test1/");
    // 规则状态，标示规则是否启用
    rule1.setStatus(StatusType::StatusEnabled);
    // 对象的过期时间
    // Days: 指定在对象过期多少天后进行操作，需要传入参数为 int 类型，当前传入为 int 类型
    // Date: 指定对象在哪一天过期，需要传入参数为 time_t 类型
    rule1.setExpiratioon(std::make_shared<Expiration>(60));
    // 设置对象标签
    rule1.addTag(Tag("key1", "value1"));
    rule1.addTag(Tag("key2", "value2"));

    // 指定生命周期规则 rule2，该规则设置创建 365 天后过期
    auto rule2 = LifecycleRule();
    // 规则 ID
    rule2.setId("rule2");
    // 指定规则适用的前缀，匹配前缀
    rule2.setPrefix("test2/");
    // 规则状态，标示规则是否启用
    rule2.setStatus(StatusType::StatusDisabled);
    // 获取 time_t 类型的当前时间
    time_t expirationTime = time(nullptr);
    tm* gmtmExpiration = gmtime(&expirationTime);
    gmtmExpiration->tm_min = 0;
    gmtmExpiration->tm_hour = 0;
    gmtmExpiration->tm_sec = 0;
    gmtmExpiration->tm_year = gmtmExpiration->tm_year + 1;
    auto exactTime = timegm(gmtmExpiration);
    // 对象的过期时间
    // Days: 指定在对象过期多少天后进行操作，需要传入参数为 int 类型
    // Date: 指定对象在哪一天过期，需要传入参数为 time_t 类型，当前传入为 time_t 类型
    rule2.setExpiratioon(exactTime);

    // 指定生命周期规则 rule3，为针对版本控制状态下的Bucket的生命周期规则。
    auto rule3 = LifecycleRule();
    rule3.setId("rule3");
    rule3.setPrefix("test3/");
    rule3.setStatus(StatusType::StatusDisabled);

    // 设置当前版本的 Object 距其最后修改时间365天之后自动转为归档类型。
    auto transition = Transition();
    transition.setDays(365);
    transition.setStorageClass(StorageClassType::ARCHIVE_FR);
    rule3.addTransition(transition);

    // 设置非当前版本的Object距最后修改时间10天之后转为低频访问类型。
    auto transition1 = NoncurrentVersionTransition(10, StorageClassType::IA);

    // 设置非当前版本的Object距最后修改时间20天之后转为归档类型。
    auto transition2 = NoncurrentVersionTransition(20, StorageClassType::ARCHIVE_FR);

    // 设置Object在其成为非当前版本30天之后删除。
    rule3.setNoncurrentVersionExpiration(30);

    std::vector<NoncurrentVersionTransition> noncurrentVersionStorageTransitions{transition1, transition2};
    rule3.setNoncurrentVersionTransitions(noncurrentVersionStorageTransitions);

    // 设置生命周期规则，包含 rule1、rule2、rule3。
    std::vector<LifecycleRule> rules{rule1, rule2, rule3};
    input.setRules(rules);

    auto output = client.putBucketLifecycle(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutBucketLifecycle failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutBucketLifecycle success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::GetBucketLifecycle() {
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

    // 查询桶生命周期规则
    GetBucketLifecycleInput input(bucketName);
    auto output = client.getBucketLifecycle(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketLifecycle failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    // 打印桶的生命周期
    auto lifycycleRules = output.result().getRules();
    for (auto& rule : lifycycleRules) {
        // 查看规则 ID、匹配前缀、规则状态(标识规则是否启用)、对象过期时间
        std::cout << "rule:" << rule.getId() << "," << rule.getPrefix() << "," << rule.getStringFormatStatus() << ","
                  << "hasExpiration:" << rule.hasExpiration() << "," << std::endl;

        // 查看当前规则的 tags
        auto taglist = rule.getTags();
        for (const auto& tag : taglist) {
            std::cout << "GetBucketLifecycle tag success, Key:" << tag.getKey() << "; Value:" << tag.getValue()
                      << std::endl;
        }

        // 查看当前版本 Object 存储类型转换规则。
        for (auto const lifeCycleTransition : rule.getTransitions()) {
            std::cout << "rule versions trans days:" << lifeCycleTransition.getDays()
                      << " trans storage class: " << lifeCycleTransition.getStringFormatStorageClass() << std::endl;
        }

        // 查看非当前版本 Object 存储类型转换规则。
        for (auto const lifeCycleTransition : rule.getNoncurrentVersionTransitions()) {
            std::cout << "rule noncurrent versions trans days:" << lifeCycleTransition.getNoncurrentDays()
                      << " trans storage class: " << lifeCycleTransition.getStringFormatStorageClass() << std::endl;
        }

        // 查看当前版本Object过期规则。
        if (rule.hasExpiration()) {
            std::cout << "rule versions expiration days:" << rule.getExpiratioon()->getDays() << std::endl;
        }

        // 查看非当前版本Object过期规则。
        if (rule.hasNoncurrentVersionExpiration()) {
            std::cout << "rule noncurrent versions expiration days:"
                      << rule.getNoncurrentVersionExpiration()->getNoncurrentDays() << std::endl;
        }

        // 查看 CompleteMultipartUpload 终止规则
        if (rule.hasAbortInCompleteMultipartUpload()) {
            std::cout << "abort in completeMultipartUpload days:"
                      << rule.getAbortInCompleteMultipartUpload()->getDaysAfterInitiation() << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::DeleteBucketLifecycle() {
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

    // 删除桶的生命周期
    DeleteBucketLifecycleInput input(bucketName);
    auto output = client.deleteBucketLifecycle(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteBucketLifecycle failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DeleteBucketLifecycle success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

// 镜像回源
int BucketSample::PutBucketMirrorBack() {
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

    // 设置桶的镜像回源
    PutBucketMirrorBackInput input(bucketName);
    // 设置镜像回源规则 rule1
    MirrorBackRule rule1;
    // 设置镜像回源 Condition: 触发回源规则的错误码，目前只支持 404
    auto condition = rule1.getCondition();
    condition.setHttpCode(404);
    rule1.setCondition(condition);

    // 设置镜像回源 Redirect:
    auto redirect = Redirect();
    // 指定跳转的类型。
    redirect.setRedirectType(RedirectType::RedirectMirror);
    // 指定镜像回源的源站地址。例如https://www.example.com/。
    SourceEndpoint endpoint;
    endpoint.setPrimary({"https://www.example.com/"});
    endpoint.setFollower({"https://www.example.com/"});
    PublicSource publicSource(endpoint);
    redirect.setPublicSource(publicSource);

    // 执行跳转时是否携带请求参数
    redirect.setPassQuery(true);
    // 镜像回源结果是 3XX 时， 是否跳转到 Location 获取数据
    redirect.setFollowRedirect(true);
    // 重定向后是否去配置源站拉取数据
    redirect.setFetchSourceOnRedirect(true);

    // 设置MirrorHeader，表示允许和禁止透传哪些Header到源站
    // 是否透传除以下Header之外的其他Header到源站。只有设置RedirectType为Mirror时生效。
    MirrorHeader mirrorHeaders;
    mirrorHeaders.setPassAll(false);
    // 透传指定的Header到源站。只有设置RedirectType为Mirror时生效。
    mirrorHeaders.setPass({"header-1", "header-2"});
    // 禁止透传指定的Header到源站。只有设置RedirectType为Mirror时生效。
    mirrorHeaders.setRemove({"header-3", "header-4"});
    redirect.setMirrorHeader(mirrorHeaders);

    // 设置镜像回源规则。
    std::vector<MirrorBackRule> rules{rule1};
    input.setRules(rules);

    auto output = client.putBucketMirrorBack(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutBucketMirrorBack failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutBucketMirrorBack success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::GetBucketMirrorBack() {
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

    // 获取桶的镜像回源配置
    GetBucketMirrorBackInput input(bucketName);
    auto output = client.getBucketMirrorBack(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketMirrorBack failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    // 打印桶的镜像回源配置
    auto mirrorBackRules = output.result().getRules();
    for (auto& rule : mirrorBackRules) {
        // 打印规则 ID
        std::cout << "rule:" << rule.getId() << std::endl;

        // 打印 conditon，查看触发回源规则的错误码，目前只支持 404
        auto conditon = rule.getCondition();
        std::cout << "http code conditon:" << conditon.getHttpCode() << std::endl;

        // 打印
        // redirect，查看回源规则跳转类型、重定向后是否去配置源站拉取数据、执行跳转时是否携带请求参数、镜像回源结果是
        // 3XX 时， 是否跳转到 Location 获取数据
        auto redirect = rule.getRedirect();
        std::cout << "redirect:" << redirect.getStringFormatRedirectType() << "," << redirect.isFetchSourceOnRedirect()
                  << "," << redirect.isPassQuery() << "," << redirect.isFollowRedirect() << std::endl;

        // 打印 mirrorHeader，查看是否透传全部 HTTP HEADER 到源端、透传和禁止透传的 HEADER
        auto mirrorHeader = redirect.getMirrorHeader();
        std::cout << "redirect-mirrorHeader:" << mirrorHeader.isPassAll() << std::endl;
        std::cout << "redirect-mirrorHeader-pass:" << std::endl;
        for (auto& pass : mirrorHeader.getPass()) {
            std::cout << pass << std::endl;
        }
        std::cout << "redirect-mirrorHeader-remove:" << std::endl;
        for (auto& remove : mirrorHeader.getRemove()) {
            std::cout << remove << std::endl;
        }
        // 打印可访问的源端地址
        auto publicSource = redirect.getPublicSource();
        auto sourceEndpoint = publicSource.getSourceEndpoint();
        std::cout << "redirect-publicSource-sourceEndpoint:" << std::endl;
        std::cout << "redirect-publicSource-sourceEndpoint-primary:" << std::endl;
        for (auto& primary : sourceEndpoint.getPrimary()) {
            std::cout << primary << std::endl;
        }
        std::cout << "redirect-publicSource-sourceEndpoint-follower:" << std::endl;
        for (auto& follower : sourceEndpoint.getFollower()) {
            std::cout << follower << std::endl;
        }
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::DeleteBucketMirrorBack() {
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

    // 删除桶的镜像回源配置
    DeleteBucketMirrorBackInput input(bucketName);
    auto output = client.deleteBucketMirrorBack(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteBucketMirrorBack failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DeleteBucketMirrorBack success." << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

// 跨域资源共享配置
int BucketSample::PutBucketCORS() {
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

    // 设置桶的跨域资源共享
    PutBucketCORSInput input(bucketName);
    // 设置跨域资源共享规则
    CORSRule rule1;
    // 指定允许跨域请求的来源。* 表示允许所有请求。
    rule1.addAllowedOrigin("*");
    // 指定允许跨域请求方法（GET/PUT/POST/DELETE/HEAD）
    rule1.setAllowedMethods({"GET", "PUT"});
    // 设置请求可以使用哪些自定义的 HTTP 请求头部
    rule1.setAllowedHeaders({"header1", "header2"});
    // 设置浏览器可以接收到的来自服务器端的自定义头部信息
    rule1.setExposeHeaders({"tos-1", "tos-2"});
    // 设置 OPTIONS 请求得到结果的有效期
    rule1.setMaxAgeSeconds(100);

    auto rule2 = CORSRule();
    rule2.setAllowedOrigins({"http://example.com", "http://www.volcengine.com"});
    rule2.addAllowedMethod("GET");
    rule2.addAllowedHeader("header1");
    rule2.addExposeHeader("tos-1");
    rule2.setMaxAgeSeconds(100);

    // 设置跨域资源共享规则
    std::vector<CORSRule> rules{rule1, rule2};
    input.setRules(rules);

    auto output = client.putBucketCORS(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutBucketCORS failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutBucketCORS success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::GetBucketCORS() {
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

    // 获取桶的跨域资源共享规则
    GetBucketCORSInput input(bucketName);
    auto output = client.getBucketCORS(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketCors failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }

    std::cout << "GetBucketCors success" << std::endl;
    // 查看跨域访问规则详细信息
    for (const auto& rule : output.result().getRules()) {
        std::cout << "Get Bucket Cors List:" << std::endl;
        for (auto const& origin : rule.getAllowedOrigins()) {
            std::cout << "Allowed origin:" << origin << std::endl;
        }
        for (auto const& header : rule.getAllowedHeaders()) {
            std::cout << "Allowed header:" << header << std::endl;
        }
        for (auto const& header : rule.getExposeHeaders()) {
            std::cout << "Expose header:" << header << std::endl;
        }
        for (auto const& method : rule.getAllowedMethods()) {
            std::cout << "Allowed method:" << method << std::endl;
        }
        std::cout << "Allowed method:" << rule.getMaxAgeSeconds() << std::endl;
    }

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::DeleteBucketCORS() {
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

    // 删除桶的跨域资源共享
    DeleteBucketCORSInput input(bucketName);
    auto output = client.deleteBucketCORS(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteBucketCORS failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DeleteBucketCORS success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::OpenBucketVersioning() {
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
    PutBucketVersioningInput input(bucketName, VersioningStatusType::Enabled);

    auto output = client.putBucketVersioning(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "OpenBucketVersioning failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "OpenBucketVersioning success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::SuspendBucketVersioning() {
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
    PutBucketVersioningInput input(bucketName, VersioningStatusType::Suspended);

    auto output = client.putBucketVersioning(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "SuspendBucketVersioning failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "SuspendBucketVersioning success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::GetBucketVersioning() {
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
    GetBucketVersioningInput input(bucketName);

    auto output = client.getBucketVersioning(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketVersioning failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "GetBucketVersioning success." << std::endl;
    std::cout << "versioning statue" << output.result().getStringFormatVersioningStatus() << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::PutBucketNotification() {
    // 初始化 TOS 账号信息
    // Your Region 填写 Bucket 所在 Region
    std::string region = "Your Region";
    std::string accessKey = "Your Access Key";
    std::string secretKey = "Your Secret Key";
    // 填写 Bucket 名称，例如 examplebucket
    std::string bucketName = "examplebucket";
    // 填写目标函数服务 cloudFunction
    std::string cloudFunction = "your cloud function";

    // 初始化网络等资源
    InitializeClient();
    // 创建交互的 client
    TosClientV2 client(region, accessKey, secretKey);

    PutBucketNotificationInput input(bucketName);

    // 设置事件通知的过滤规则
    // 如果前后缀均未设置，则会匹配存储桶内所有对象。
    FilterKey filterKey;
    // 设置匹配对象的前缀的信息为 test/，会筛选 test 目录下的对象。
    FilterRule rule1({"prefix", "test/"});
    // 设置匹配对象的后缀的信息为 .png，会筛选所有格式为 png 的对象。
    FilterRule rule2({"suffix", ".png"});
    filterKey.setRules({rule1, rule2});
    Filter filter(filterKey);

    CloudFunctionConfiguration configuration;
    // 设置 id
    configuration.setId("1");
    // 设置事件类型
    configuration.setEvents({"tos:ObjectCreated:Put"});
    // 设置目标函数服务
    configuration.setCloudFunction(cloudFunction);
    // 设置过滤规则
    configuration.setFilter(filter);
    input.setCloudFunctionConfigurations({configuration});

    RocketMQConfiguration rocketMqConfiguration;
    rocketMqConfiguration.setId("2");
    rocketMqConfiguration.setEvents({"tos:ObjectCreated:Post"});
    rocketMqConfiguration.setFilter(filter);
    rocketMqConfiguration.setRocketMq(RocketMQConf("your mq instance id", "your mq topic", "your mq access key id"));
    input.setRocketMqConfigurations({rocketMqConfiguration});

    auto output = client.putBucketNotification(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutBucketNotification failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutBucketNotification success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
int BucketSample::GetBucketNotification() {
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
    GetBucketNotificationInput input(bucketName);

    auto output = client.getBucketNotification(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketNotification failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "GetBucketNotification success." << std::endl;
    for (const auto& config : output.result().getCloudFunctionConfigurations()) {
        std::cout << "id:" << config.getId() << std::endl;
        std::cout << "cloud function:" << config.getCloudFunction() << std::endl;
        auto rules = config.getFilter().getKey().getRules();
        for (const auto& rule : rules) {
            std::cout << "filter name:" << rule.getName() << std::endl;
            std::cout << "filter value:" << rule.getValue() << std::endl;
        }
        for (const auto& event : config.getEvents()) {
            std::cout << "event:" << event << std::endl;
        }
    }
    for (const auto& config : output.result().getRocketMqConfigurations()) {
        std::cout << "id:" << config.getId() << std::endl;
        const auto& mqConf = config.getRocketMq();
        std::cout << "rocket mq instance id:" << mqConf.getInstanceId() << std::endl;
        std::cout << "rocket mq topic:" << mqConf.getTopic() << std::endl;
        std::cout << "rocket mq accessKey id:" << mqConf.getAccessKeyId() << std::endl;

        auto rules = config.getFilter().getKey().getRules();
        for (const auto& rule : rules) {
            std::cout << "filter name:" << rule.getName() << std::endl;
            std::cout << "filter value:" << rule.getValue() << std::endl;
        }
        for (const auto& event : config.getEvents()) {
            std::cout << "event:" << event << std::endl;
        }
    }
    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::PutBucketRename() {
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

    // 设置桶的策略配置
    PutBucketRenameInput input(bucketName, true);
    auto output = client.putBucketRename(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "PutBucketRename failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "PutBucketRename success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::GetBucketRename() {
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

    GetBucketRenameInput input(bucketName);
    auto output = client.getBucketRename(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "GetBucketRename failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "GetBucketRename success" << std::endl;
    // 释放网络等资源
    CloseClient();
    return 0;
}

int BucketSample::DeleteBucketRename() {
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

    DeleteBucketRenameInput input(bucketName);
    auto output = client.deleteBucketRename(input);
    if (!output.isSuccess()) {
        // 异常处理
        std::cout << "DeleteBucketRename failed." << output.error().String() << std::endl;
        // 释放网络等资源
        CloseClient();
        return -1;
    }
    std::cout << "DeleteBucketRename success." << std::endl;

    // 释放网络等资源
    CloseClient();
    return 0;
}
