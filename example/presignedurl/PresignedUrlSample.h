#include "TosClientV2.h"

class PresignedUrlSample {
public:
    PresignedUrlSample();
    ~PresignedUrlSample();

    void GenPutPreSignedUrl();
    void GenGetPreSignedUrl();
    void GenPreSignedPostSigned();
    void GenPreSignedPostSignedWithMultiForm();
};
