#include "model/bucket/ListBucketsOutput.h"
#include "../src/external/json/json.hpp"
using namespace nlohmann;

using namespace VolcengineTos;
void ListBucketsOutput::fromJsonString(const std::string &output) {
  json j = json::parse(output);
  json bkts = j.at("Buckets");
  for (auto & bkt : bkts) {
    ListedBucket lb;
    lb.fromJsonString(bkt);
    buckets_.emplace_back(lb);
  }
  std::string id;
  if (j.contains("Owner") && j.at("Owner").contains("ID")) {
    j.at("Owner").at("ID").get_to(id);
  }
  owner_.setId(id);
}