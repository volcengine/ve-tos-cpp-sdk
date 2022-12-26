#include "TosClientV2.h"

class DirectorySample {
public:
    DirectorySample();
    ~DirectorySample();

    int PutFolder();
    int PutMultiLevelFolder();
    int ListFolder();
    int DeleteFolder();
};
