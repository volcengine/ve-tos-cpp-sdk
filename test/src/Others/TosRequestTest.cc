#include <gtest/gtest.h>
#include "RequestBuilder.h"
#include "utils/BaseUtils.h"
using namespace VolcengineTos;

static std::string findHeaderIgnoreCase(const std::map<std::string, std::string>& headers, const std::string& key) {
    auto lowerKey = StringUtils::toLower(key);
    for (const auto& header : headers) {
        if (StringUtils::toLower(header.first) == lowerKey) {
            return header.second;
        }
    }
    return {};
}

TEST(TosRequestTest, RequestURLTest) {
    //        /abc/😊?/😭#~!.txt
    //  const char16_t* path = u"/abc/\u16D83D\u16DE0A?/\u16D83D\u16DE2D#~!.txt";
    //  std::wstring_convert<std::codecvt<char16_t, char, mbstate_t>, char16_t> conv;
    //
    //  TosRequest req("http", "GET", "localhost", conv.to_bytes(path));
    //  auto query = std::map<std::string, std::string>();
    //  query.emplace("versionId", "abc123");
    //  req.setQueries(query);
    //  auto u = req.toUrl().toString();
    //  EXPECT_EQ(u, "https://localhost/abc/%F0%9F%98%8A%3F/%F0%9F%98%AD%23~!.txt?versionId=abc123");
}

TEST(TosRequestTest, GenericInputRequestHeaderMergeTest) {
    std::map<std::string, std::string> headers = {{"User-Agent", "sdk-agent"}, {"Content-Type", "text/plain"}};
    std::map<std::string, std::string> queries;
    RequestBuilder rb(nullptr, "https", "tos-cn-beijing.ivolces.com", "", "", "bucket", "object", 0, headers, queries,
                      false);

    rb.setRequestHeader({{"x-tos-meta-custom", "custom-value"},
                         {"X-Custom-Header", "custom"},
                         {"content-type", "application/json"},
                         {"Connection", "close"},
                         {"Authorization", "ignored"}});

    auto req = rb.build("GET");
    const auto& mergedHeaders = req->getHeaders();

    EXPECT_EQ("custom-value", findHeaderIgnoreCase(mergedHeaders, "x-tos-meta-custom"));
    EXPECT_EQ("custom", findHeaderIgnoreCase(mergedHeaders, "x-custom-header"));
    EXPECT_EQ("text/plain", findHeaderIgnoreCase(mergedHeaders, "content-type"));
    EXPECT_TRUE(findHeaderIgnoreCase(mergedHeaders, "connection").empty());
    EXPECT_TRUE(findHeaderIgnoreCase(mergedHeaders, "authorization").empty());
}
