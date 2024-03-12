#include "BoardCtrl.h"
#include <ctime>
#include <iomanip>

#define BOARD_ELEM_COMMENT "comment"
#define USERS_DBCL_NAME "users"

void BoardCtrl::access(const HttpRequestPtr &req,
                       std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << endl
              << req->getPeerAddr().toIp() << endl;

    HttpResponsePtr resp;
    if (req->session()->getOptional<bool>(SESSION_LOGGED_IN).value_or(false))
    {
        resp = HttpResponse::newRedirectionResponse(INDEX);
    }
    else
    {
        resp = HttpResponse::newHttpResponse();
        resp->setBody(file_to_string("pages/login.html"));
    }
    callback(resp);
}

void BoardCtrl::login(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << endl;

    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    std::string username = req->getParameter(SESSION_USERNAME);
    std::string password = req->getParameter(SESSION_PASSWORD);

    bool found = false;
    auto usersDbClientPtr = drogon::app().getDbClient(USERS_DBCL_NAME);
    auto fObj = usersDbClientPtr->execSqlAsyncFuture("select * from users where username=$1", username);
    try
    {
        auto result = fObj.get();
        for (auto row : result)
        {
            if (row[SESSION_USERNAME].as<std::string>() == username &&
                row[SESSION_PASSWORD].as<std::string>() == sha256(password))
            {
                found = true;
                break;
            }
        }
    }
    catch (const drogon::orm::DrogonDbException &e)
    {
        LOG_DEBUG << "error:" << e.base().what() << endl;
    }

    if (found)
    {
        resp = accept_login(username, req);
    }
    else
    {
        resp = deny_login();
    }
    callback(resp);
}

void BoardCtrl::get_reg(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << endl;
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    resp->setBody(file_to_string("pages/reg.html"));
    callback(resp);
}

void BoardCtrl::reg(const HttpRequestPtr &req,
                    std::function<void(const HttpResponsePtr &)> &&callback)
{
    MultiPartParser parser;
    LOG_DEBUG << "pareser result: " << parser.parse(req) << endl;

    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    ;
    std::string pfp = parser.getParameter<std::string>("pfp");
    std::string username = parser.getParameter<std::string>(SESSION_USERNAME);
    std::string password = parser.getParameter<std::string>(SESSION_PASSWORD);
    LOG_DEBUG << "pfp: " << pfp.length() << ", username: " << username << ", password: " << password << endl;

    if (username == "" || password == "")
    {
        LOG_DEBUG << "Empty creds specified, username: " << username << endl;
        callback(deny_login());
        return;
    }

    bool found = false;
    auto usersDbClientPtr = drogon::app().getDbClient(USERS_DBCL_NAME);
    auto fObj = usersDbClientPtr->execSqlAsyncFuture("select * from users where username=$1", username);

    try
    {
        auto result = fObj.get();
        found = result.size();
    }
    catch (const drogon::orm::DrogonDbException &e)
    {
        LOG_DEBUG << "error:" << e.base().what() << endl;
    }
    if (found)
    {
        LOG_DEBUG << "User already exists" << endl;
        callback(deny_login());
        return;
    }
    else
    {
        auto fObj = usersDbClientPtr->execSqlAsyncFuture("insert into users (username, password, pfp) values ($1, $2, $3);",
                                                         username, sha256(password), pfp);
        try
        {
            auto result = fObj.get();
            resp = accept_login(username, req);
        }
        catch (const drogon::orm::DrogonDbException &e)
        {
            LOG_DEBUG << "error:" << e.base().what() << endl;
        }
    }
    callback(resp);
}

HttpResponsePtr BoardCtrl::accept_login(const std::string &username, const HttpRequestPtr &req)
{
    HttpResponsePtr resp = HttpResponse::newRedirectionResponse(INDEX);
    req->session()->insert(SESSION_USERNAME, username);
    req->session()->insert(SESSION_LOGGED_IN, true);
    return resp;
}

HttpResponsePtr BoardCtrl::deny_login()
{
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k401Unauthorized);
    resp->setBody(file_to_string("pages/invalid_login.html"));
    return resp;
}

void BoardCtrl::logout(const HttpRequestPtr &req,
                       std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << endl
              << "req->body(): " << req->body() << endl;

    HttpResponsePtr resp = HttpResponse::newRedirectionResponse(ROOT);
    req->session()->clear();
    callback(resp);
}

void BoardCtrl::index(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << endl
              << "req->body(): " << req->body() << endl;

    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    resp->setBody(file_to_string("index.html"));
    callback(resp);
}

void BoardCtrl::get_comments(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback,
                             int num, int offset, const std::string &comp, const std::string &order)
{
    LOG_DEBUG << endl
              << "req->body(): " << req->body() << endl;
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    std::string curr_username = req->session()->get<std::string>(SESSION_USERNAME);
    std::string str_offset = std::to_string(offset);
    std::string str_num = std::to_string(num);
    std::string str_comp = comp;
    std::string comments;

    if (str_comp == "le")
    {
        str_comp = "<=";
    }
    else if (str_comp == "ge")
    {
        str_comp = ">=";
    }

    auto usersDbClientPtr = drogon::app().getDbClient(USERS_DBCL_NAME);

    auto fObj = usersDbClientPtr->execSqlAsyncFuture("select * from comments where id " + str_comp + str_offset +
                                                     " order by id " + order + " limit $1;", str_num);

    try
    {
        auto result = fObj.get();
        for (auto row : result)
        {
            std::string username = row["username"].as<std::string>();
            std::string username_style;
            std::string comment;
            if (username == curr_username)
            {
                username_style = "curr-username";
            }
            else
            {
                username_style = "username";
            }
            comment = static_cast<std::string>("<div class=comment> <div class=\"comment-header\">") +
                      "<span  class=\"" + username_style + "\"><b>" + username + "</b></span>\n" +
                      "<span>" + row["date_time"].as<std::string>() + " GMT</span>" +
                      "   #<span class=\"comment-id\">" + row["id"].as<std::string>() + "</span></div> <br>" +
                      "<div class=\"wrapper\"><image class=\"pfp box11\"/><span class=\"comment-content box12\">" +
                      row["comment"].as<std::string>() + "</div> </div>";
            if (order == "desc")
            {
                comments += comment;
            }
            else
            {
                comments = comment + comments;
            }
        }
        resp->setBody(comments);
    }
    catch (const drogon::orm::DrogonDbException &e)
    {
        LOG_DEBUG << "error:" << e.base().what() << endl;
        resp->setStatusCode(k400BadRequest);
    }
    callback(resp);
}

void BoardCtrl::get_pfp(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback,
                        const std::string &username)
{
    LOG_DEBUG << username << endl;

    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    auto usersDbClientPtr = drogon::app().getDbClient(USERS_DBCL_NAME);
    auto fObj = usersDbClientPtr->execSqlAsyncFuture("select pfp from users where username=$1",
                                                     username);
    try
    {
        auto result = fObj.get();
        resp->setBody(result.front()["pfp"].as<std::string>());
    }
    catch (const drogon::orm::DrogonDbException &e)
    {
        LOG_DEBUG << "error:" << e.base().what() << endl;
        resp->setStatusCode(k404NotFound);
    }
    callback(resp);
}

void BoardCtrl::edit_profile(const HttpRequestPtr &req,
                           std::function<void(const HttpResponsePtr &)> &&callback)
{
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    resp->setBody(file_to_string("pages/edit_profile.html"));
    callback(resp);
}

void BoardCtrl::change_pfp(const HttpRequestPtr &req,
                           std::function<void(const HttpResponsePtr &)> &&callback)
{
    MultiPartParser parser;
    LOG_DEBUG << "pareser result: " << parser.parse(req) << endl;

    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    std::string username = req->session()->get<std::string>(SESSION_USERNAME);
    std::string pfp = parser.getParameter<std::string>("pfp");
    LOG_DEBUG << username<<", pfp sz in B"<<pfp.length() << endl;
    auto usersDbClientPtr = drogon::app().getDbClient(USERS_DBCL_NAME);
    auto fObj = usersDbClientPtr->execSqlAsyncFuture("update users set pfp = $1 where username=$2", 
                                                     pfp, username);
    try
    {
        auto result = fObj.get();
        LOG_DEBUG<<"rows affected: "<<result.affectedRows()<<endl;
    }
    catch (const drogon::orm::DrogonDbException &e)
    {
        LOG_DEBUG << "error:" << e.base().what() << endl;
        resp->setStatusCode(k406NotAcceptable);
    }
    callback(resp);
}

void BoardCtrl::post_comment(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << endl
              << "req->body(): " << req->body() << endl;
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    std::string username = req->session()->get<std::string>(SESSION_USERNAME);
    std::string_view comment = req->getBody();
    std::cout << comment << endl;
    std::time_t time = std::time(nullptr);
    std::stringstream timestamp;
    timestamp << std::put_time(std::gmtime(&time), "%Y-%m-%d %H:%M:%S %Z");
    auto usersDbClientPtr = drogon::app().getDbClient(USERS_DBCL_NAME);
    auto fObj = usersDbClientPtr->execSqlAsyncFuture("insert into comments (username, date_time, comment) values ($1, $2, $3);",
                                                     username, timestamp.str(), comment);

    try
    {
        auto result = fObj.get();
        resp->setStatusCode(k202Accepted);
    }
    catch (const drogon::orm::DrogonDbException &e)
    {
        LOG_DEBUG << "error:" << e.base().what() << endl;
        resp->setStatusCode(k406NotAcceptable);
    }
    callback(resp);
}
