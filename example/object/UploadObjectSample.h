#include "TosClientV2.h"

class UploadObjectSample {
public:
    UploadObjectSample();
    ~UploadObjectSample();

    // 普通上传对象
    int PutObjectFromBuffer();
    int PutObjectFromFileContent();
    int PutObjectFromFile();
    int PutObjectWithMetaData();
    int PutObjectWithProcess();
    int PutObjectWithRateLimiter();
    int PutObjectFromFolder();
    // 追加上传
    int AppendObjectFromBuffer();
    int AppendObjectWithProcess();
    int AppendObjectWithRateLimiter();
    // 分片上传
    int MultipartUpload();
    int ListParts();
    int AbortMultipartUpload();
    int MultipartUploadWithProcess();
    int MultipartUploadWithRateLimiter();
    // 断点续传上传
    int UploadFile();
    int UploadFileWithProcess();
    int UploadFileWithEvent();
    int UploadFileWithRateLimiter();
    int UploadFileWithCancel();
};
