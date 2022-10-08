#pragma once

#include <vector>
#include <string>
namespace VolcengineTos {
class MirrorHeader {
public:
    bool isPassAll() const {
        return passAll_;
    }
    void setPassAll(bool passAll) {
        passAll_ = passAll;
    }
    const std::vector<std::string>& getPass() const {
        return pass_;
    }
    void setPass(const std::vector<std::string>& pass) {
        pass_ = pass;
    }
    const std::vector<std::string>& getRemove() const {
        return remove_;
    }
    void setRemove(const std::vector<std::string>& remove) {
        remove_ = remove;
    }

private:
    bool passAll_;
    std::vector<std::string> pass_;
    std::vector<std::string> remove_;
};
}  // namespace VolcengineTos
