#include "model/acl/PreSignedPolicyURLOutput.h"
#include "../src/auth/SignV4.h"

std::string VolcengineTos::PreSignedPolicyURLOutput::queryToString(
        const std::map<std::string, std::string>& additioQuery) {
    if (additioQuery.empty()) {
        return "";
    }
    std::string res;
    for (const auto& q : additioQuery) {
        res += SignV4::uriEncode(q.first, true);
        res += "=";
        res += SignV4::uriEncode(q.second, true);
        ;
        res += "&";
    }
    res = res.substr(0, res.size() - 1);

    return res;
}
