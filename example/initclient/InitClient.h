#include "TosClientV2.h"

class InitClient {
public:
    InitClient();
    ~InitClient();

    void InitClientWithDefaultConfig();
    void InitClientWithSTS();
    void InitClientWithTimeout();
    void InitClientWithRetry();
    void InitClientWithCrc();
    void InitClientWithDnsCache();
    void InitClientWithProxy();
    void InitClientWithAllConfig();
};
