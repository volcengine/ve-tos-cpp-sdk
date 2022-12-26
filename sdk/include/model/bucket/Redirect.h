#pragma once

#include <string>
#include <utility>
#include "Type.h"
#include "MirrorHeader.h"
#include "PublicSource.h"
namespace VolcengineTos {
class Redirect {
public:
    Redirect() = default;
    virtual ~Redirect() = default;
    Redirect(RedirectType redirectType, bool fetchSourceOnRedirect, bool passQuery, bool followRedirect,
             const MirrorHeader& mirrorHeader, const PublicSource& publicSource)
            : redirectType_(redirectType),
              fetchSourceOnRedirect_(fetchSourceOnRedirect),
              passQuery_(passQuery),
              followRedirect_(followRedirect),
              mirrorHeader_(mirrorHeader),
              publicSource_(publicSource) {
    }
    RedirectType getRedirectType() const {
        return redirectType_;
    }
    void setRedirectType(RedirectType redirectType) {
        redirectType_ = redirectType;
    }
    bool isFetchSourceOnRedirect() const {
        return fetchSourceOnRedirect_;
    }
    void setFetchSourceOnRedirect(bool fetchSourceOnRedirect) {
        fetchSourceOnRedirect_ = fetchSourceOnRedirect;
    }
    bool isPassQuery() const {
        return passQuery_;
    }
    void setPassQuery(bool passQuery) {
        passQuery_ = passQuery;
    }
    bool isFollowRedirect() const {
        return followRedirect_;
    }
    void setFollowRedirect(bool followRedirect) {
        followRedirect_ = followRedirect;
    }
    const MirrorHeader& getMirrorHeader() const {
        return mirrorHeader_;
    }
    void setMirrorHeader(const MirrorHeader& mirrorHeader) {
        mirrorHeader_ = mirrorHeader;
    }
    const PublicSource& getPublicSource() const {
        return publicSource_;
    }
    void setPublicSource(const PublicSource& publicSource) {
        publicSource_ = publicSource;
    }
    const std::string& getStringFormatRedirectType() const {
        return RedirectTypetoString[redirectType_];
    }

private:
    RedirectType redirectType_ = RedirectType::NotSet;
    bool fetchSourceOnRedirect_ = false;
    bool passQuery_ = false;
    bool followRedirect_ = false;
    MirrorHeader mirrorHeader_;
    PublicSource publicSource_;
};
}  // namespace VolcengineTos
