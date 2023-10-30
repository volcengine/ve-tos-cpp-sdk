#include "TosClientV2.h"

class OthersObjectSample {
public:
    OthersObjectSample();
    ~OthersObjectSample();

    // 管理对象元数据
    int PutObjectWithMetaData();
    int MultipartUploadWithMetaData();
    int SetObjectMeta();
    int GetObjectMeta();
    int DoesObjectExist();

    // 管理对象读写权限
    int PutObjectWithAcl();
    int MultipartUploadWithAcl();
    int PutObjectAcl();
    // 通过 grant 设置对象读写权限
    int SetObjectACLWithGrant();
    // 通过详细配置设置桶读写权限配置
    int SetObjectACLByDetailedConf();
    int GetObjectAcl();

    // 删除单对象
    int DeleteObject();
    // 删除带版本对象
    int DeleteObjectVersioning();
    // 批量删除对象
    int BatchDeleteObject();
    // 批量删除带版本对象
    int BatchDeleteObjectVersioning();
    // 结合列举对象接口删除桶内对象
    int ListAndDeleteObjects();
    // 结合列举对象接口删除桶内对象，删除所有未合并对象
    int ListAndDeleteObjects2();
    // 结合列举对象接口删除桶内对象，删除所有未合并对象
    int ListAndDeleteObjectsVersioning();
    // 管理对象标签
    int PutObjectTagging();
    int GetObjectTagging();
    int DeleteObjectTagging();

    int PutObjectWithErrorProcess();

    // 解冻对象
    int RestoreObject();
};
