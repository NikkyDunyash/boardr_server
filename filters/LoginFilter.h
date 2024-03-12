#pragma once

#include <drogon/HttpFilter.h>
#include "utils.h"
using namespace drogon;


class LoginFilter : public HttpFilter<LoginFilter>
{
  public:
    LoginFilter() {}
    void doFilter(const HttpRequestPtr &req,
                  FilterCallback &&fcb,
                  FilterChainCallback &&fccb) override;
};

