#include "TosClientV2.h"

class ListObjectSample {
public:
    ListObjectSample();
    ~ListObjectSample();

    // 新版listObjectsType2
    int ListObjectsNew();
    int ListObjectsWithContinuationTokenNew();
    int ListObjectsWithEncodingTypeNew();
    int ListObjectsWithPrefixNew();
    int ListAllObjectsNew();
    int ListObjectsWithDelimiterNew();
    int ListAllObjectsWithDelimiterNew();

    // 旧版listObject
    int ListObjects();
    int ListObjectsWithNextMarker();
    int ListObjectsWithEncodingType();
    int ListObjectsWithPrefix();
    int ListAllObjects();
    int ListObjectsWithDelimiter();
    int ListAllObjectsWithDelimiter();

    // 列举多版本
    int ListObjectsVersions();
    int ListObjectsWithNextMarkerVersions();
    int ListObjectsWithEncodingTypeVersions();
    int ListObjectsWithPrefixVersions();
    int ListAllObjectsVersions();
    int ListObjectsWithDelimiterVersions();
    int ListAllObjectsWithDelimiterVersions();

    // 列举未合并对象
    int ListMultipartUpload();
    int ListMultipartUploadWithEncodingType();
    int ListMultipartUploadWithPrefix();
    int ListMultipartUploadsAll();
    int ListMultipartUploadWithDelimiter();
    int ListMultipartUploadAllWithDelimiter();
};
