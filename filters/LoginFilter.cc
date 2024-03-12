#include "LoginFilter.h"
#include "utils.h"

using namespace drogon;

void LoginFilter::doFilter(const HttpRequestPtr &req,
                         FilterCallback &&fcb,
                         FilterChainCallback &&fccb)
{
    if (req->session()->getOptional<bool>(SESSION_LOGGED_IN).value_or(false))
    {
        //Passed
        fccb();
        return;
    }
    //Check failed
    auto resp = HttpResponse::newRedirectionResponse(ROOT);
    fcb(resp);
}
