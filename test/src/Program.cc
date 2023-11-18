#include "TosClientV2.h"
#include <gtest/gtest.h>
#include <photon/photon.h>
#include "TestConfig.h"
#include "utils/LogUtils.h"

using namespace VolcengineTos;

int main(int argc, char** argv) {
    std::cout << "tos-cpp-sdk test" << std::endl;
    testing::InitGoogleTest(&argc, argv);
    int ret = photon::init(photon::INIT_EVENT_EPOLL, photon::INIT_IO_LIBCURL);
    if (ret != 0) {
        std::cerr << "Photon coroutine init failed" << std::endl;
        return -1;
    }
    InitializeClient();

    ret = RUN_ALL_TESTS();

    CloseClient();

    photon::fini();
    return ret;
}
