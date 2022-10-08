#include "TosClientV2.h"
#include <gtest/gtest.h>
#include "TestConfig.h"
#include "utils/LogUtils.h"

using namespace VolcengineTos;

int main(int argc, char** argv) {
    std::cout << "tos-cpp-sdk test" << std::endl;
    testing::InitGoogleTest(&argc, argv);
    InitializeClient();

    int ret = RUN_ALL_TESTS();

    CloseClient();

    return ret;
}
