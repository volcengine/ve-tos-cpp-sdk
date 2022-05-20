#include <gtest/gtest.h>
#include "../../sdk/include/utils/BaseUtils.h"
using namespace VolcengineTos;

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