#include "TosClientV2.h"

class BucketSample {
public:
    BucketSample();
    ~BucketSample();

    //
    int CreateBucket();
    int ListBuckets();
    int GetBucketMetaDate();
    int DeleteBucket();
    int GetBucketLocation();

    // 通过 ACL 设置桶读写权限配置
    int SetBucketACL();
    int SetBucketACLWithGrant();
    // 通过详细配置设置桶读写权限配置
    int SetBucketACLByDetailedConf();
    // 获取桶读写权限配置
    int GetBucketACL();

    // 授权策略配置
    int PutBucketPolicy();
    int GetBucketPolicy();
    int DeleteBucketPolicy();

    // 默认存储类型配置
    int PutBucketStorageClass();
    int GetBucketStorageClass();

    // 设置生命周期
    int PutBucketLifecycle();
    int GetBucketLifecycle();
    int DeleteBucketLifecycle();

    // 镜像回源
    int PutBucketMirrorBack();
    int GetBucketMirrorBack();
    int DeleteBucketMirrorBack();

    // 跨域资源共享配置
    int PutBucketCORS();
    int GetBucketCORS();
    int DeleteBucketCORS();

    // 开启/暂停/获取桶的多版本
    int OpenBucketVersioning();
    int SuspendBucketVersioning();
    int GetBucketVersioning();

    // 跨域资源复制配置
    // 多版本配置
    // 网站配置
    // 事件通知配置
    int PutBucketNotification();
    int GetBucketNotification();
    // 自定义域名配置
    // 实时日志配置
    // 重命名对象配置
    int PutBucketRename();
    int GetBucketRename();
    int DeleteBucketRename();
};
