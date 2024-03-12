#pragma once

#include <drogon/HttpController.h>
#include "utils.h"

using namespace drogon;

class BoardCtrl : public drogon::HttpController<BoardCtrl>
{
public:
  METHOD_LIST_BEGIN
  ADD_METHOD_TO(BoardCtrl::access, "/", Get);
  ADD_METHOD_TO(BoardCtrl::login, "/login", Post); 
  ADD_METHOD_TO(BoardCtrl::get_reg, "/get_reg", Get);
  ADD_METHOD_TO(BoardCtrl::reg, "/reg", Post);
  ADD_METHOD_TO(BoardCtrl::logout, "/logout", Get);
  ADD_METHOD_VIA_REGEX(BoardCtrl::index, "/index.*", Get, "LoginFilter");
  ADD_METHOD_TO(BoardCtrl::get_comments, "/get_comments?num={1}&offset={2}&comp={3}&order={4}", Get, "LoginFilter");
  ADD_METHOD_TO(BoardCtrl::get_pfp, "/username={username}/pfp", Get, "LoginFilter");
  ADD_METHOD_TO(BoardCtrl::edit_profile, "/edit_profile", Get, "LoginFilter");
  ADD_METHOD_TO(BoardCtrl::change_pfp, "/change_pfp", Post, "LoginFilter");
  ADD_METHOD_TO(BoardCtrl::post_comment, "/post_comment", Post, "LoginFilter");

  METHOD_LIST_END
  void access(const HttpRequestPtr &req,
              std::function<void(const HttpResponsePtr &)> &&callback);
  void login(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback);
  void get_reg(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback);
  void reg(const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback);
  void logout(const HttpRequestPtr &req,
              std::function<void(const HttpResponsePtr &)> &&callback);
  void index(const HttpRequestPtr &req,
             std::function<void(const HttpResponsePtr &)> &&callback);
  void get_comments(const HttpRequestPtr &req,
                    std::function<void(const HttpResponsePtr &)> &&callback,
                    int num, int offset, const std::string &comp, const std::string &order);
  void get_pfp(const HttpRequestPtr &req,
               std::function<void(const HttpResponsePtr &)> &&callback,
               const std::string &username);
  void edit_profile(const HttpRequestPtr &req,
                    std::function<void(const HttpResponsePtr &)> &&callback);
  void change_pfp(const HttpRequestPtr &req,
                  std::function<void(const HttpResponsePtr &)> &&callback);
  void post_comment(const HttpRequestPtr &req,
                    std::function<void(const HttpResponsePtr &)> &&callback);

  HttpResponsePtr accept_login(const std::string &username, const HttpRequestPtr &req);
  HttpResponsePtr deny_login();
};
