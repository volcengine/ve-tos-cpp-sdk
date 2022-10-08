#include <gtest/gtest.h>
#include "auth/SignV4.h"

using namespace VolcengineTos;

TEST(SignV4Test, URIEncodeTest) {
    auto out = SignV4::uriEncode("23i23+___", true);
    EXPECT_EQ("23i23%2B___", out);

    out = SignV4::uriEncode("23i23 ___", true);
    EXPECT_EQ("23i23%20___", out);

    out = SignV4::uriEncode("23i23 /___", true);
    EXPECT_EQ("23i23%20%2F___", out);

    out = SignV4::uriEncode("23i23 /___", false);
    EXPECT_EQ("23i23%20/___", out);

    out = SignV4::uriEncode("/中文测试/", true);
    EXPECT_EQ("%2F%E4%B8%AD%E6%96%87%E6%B5%8B%E8%AF%95%2F", out);
}

// int main(int argc, char **argv) {
//   printf("Running main() from %s\n", __FILE__);
//   testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }

