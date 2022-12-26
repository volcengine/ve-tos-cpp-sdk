#include "TosClientV2.h"

class DownloadObjectSample {
public:
    DownloadObjectSample();
    ~DownloadObjectSample();

    // 普通下载对象
    int GetObjectToBuffer();
    int GetObjectToBufferVersions();
    int GetObjectToFile();
    int GetObjectWithRewriteResponseHeader();
    int GetObjectWithProcess();
    int GetObjectWithRateLimiter();

    // 限定条件下载
    int GetObjectWithConditon();

    // 范围下载
    int GetObjectWithStartEnd();
    int GetObjectWithRange();
    int GetObjectWithMultipart();
    int GetObjectWithStartEndWithProcess();
    int GetObjectWithStartEndWithRateLimiter();

    // 断点续传上传
    int DownloadFile();
    int DownloadFileWithProcess();
    int DownloadFileWithEvent();
    int DownloadFileWithRateLimiter();
    int DownloadFileWithCancel();
};
