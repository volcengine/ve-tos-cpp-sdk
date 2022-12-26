#include "TosClientV2.h"

class CopyObjectSample {
public:
    CopyObjectSample();
    ~CopyObjectSample();

    int CopyObjectWithAcl();
    int CopyObjectWithSourceMeta();
    int CopyObjectWithDestMeta();

    int UploadPartCopy();
};
