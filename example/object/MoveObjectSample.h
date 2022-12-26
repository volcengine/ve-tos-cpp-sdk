#include "TosClientV2.h"

class MoveObjectSample {
public:
    MoveObjectSample(const std::string& bucket);
    ~MoveObjectSample();

    // 移动/重命名场景
    int CopyAndDeleteObject();
    int CopyAndDeleteDirectory();
};
