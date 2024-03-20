#include "BoardCtrl.h"
#include <ctime>
#include <iomanip>

#define BOARD_ELEM_COMMENT "comment"
#define DBCL_NAME "boardb"

const std::string DOC_ROOT_DIR = "../front/";
const std::string PAGES_DIR = DOC_ROOT_DIR + "pages/";

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
        resp->setBody(file_to_string(PAGES_DIR + "login.html"));
    }
    callback(resp);
}

void BoardCtrl::login(const HttpRequestPtr &req,
                      std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << endl;

    std::string username = req->getParameter(SESSION_USERNAME);
    std::string password = req->getParameter(SESSION_PASSWORD);

    auto dbClientPtr = drogon::app().getFastDbClient(DBCL_NAME);
    dbClientPtr->execSqlAsync(
        "select * from users where username='" + username + "';",
        [=](const drogon::orm::Result &result)
        {
            bool found = false;
            for (auto row : result)
            {
                if (row[SESSION_USERNAME].as<std::string>() == username &&
                    row[SESSION_PASSWORD].as<std::string>() == sha256(password))
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                callback(accept_login(username, req));
            }
            else
            {
                callback(deny_login());
            }
        },
        [=](const drogon::orm::DrogonDbException &e)
        {
            LOG_DEBUG << "error:" << e.base().what() << endl;
            callback(deny_login());
        });
}

void BoardCtrl::get_reg(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback)
{
    LOG_DEBUG << endl;
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    resp->setBody(file_to_string(PAGES_DIR + "reg.html"));
    callback(resp);
}

void BoardCtrl::reg(const HttpRequestPtr &req,
                    std::function<void(const HttpResponsePtr &)> &&callback)
{
    MultiPartParser parser;
    LOG_DEBUG << "pareser result: " << parser.parse(req) << endl;

    std::string pfp = parser.getParameter<std::string>("pfp");
    std::string username = parser.getParameter<std::string>(SESSION_USERNAME);
    std::string password = parser.getParameter<std::string>(SESSION_PASSWORD);
    LOG_DEBUG << "pfp: " << pfp.length() << ", username: " << username << endl;

    if (username == "" || password == "")
    {
        LOG_DEBUG << "Empty creds specified, username: " << username << endl;
        callback(deny_login());
        return;
    }

    auto dbClientPtr = drogon::app().getFastDbClient(DBCL_NAME);
    dbClientPtr->execSqlAsync(
        "select * from users where username='" + username + "';",
        [=](const drogon::orm::Result &result)
        {
            bool found = false;
            found = result.size();
            if (found)
            {
                LOG_DEBUG << "User already exists" << endl;
                callback(deny_login());
            }
            else
            {
                dbClientPtr->execSqlAsync(
                    "insert into users (username, password, pfp) values ($1, $2, $3);",
                    [=](const drogon::orm::Result &result)
                    {
                        callback(accept_login(username, req));
                    },
                    [=](const drogon::orm::DrogonDbException &e)
                    {
                        LOG_DEBUG << "error:" << e.base().what() << endl;
                        callback(deny_login());
                    },
                    username, sha256(password), pfp);
            }
        },
        [=](const drogon::orm::DrogonDbException &e)
        {
            LOG_DEBUG << "error:" << e.base().what() << endl;
            callback(deny_login());
        });
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
    resp->setBody(file_to_string(PAGES_DIR + "invalid_login.html"));
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
    resp->setBody(file_to_string(DOC_ROOT_DIR + "index.html"));
    callback(resp);
}

void BoardCtrl::get_comments(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback,
                             const std::string &num, const std::string &offset,
                             const std::string &comp, const std::string &order)
{
    LOG_DEBUG << endl
              << "req->body(): " << req->body() << endl;
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    std::string curr_username = req->session()->get<std::string>(SESSION_USERNAME);
    std::map<std::string, std::string> comp_map = {{"l", "<"}, {"le", "<="}, {"ge", ">="}, {"g", ">"}};

    auto dbClientPtr = drogon::app().getFastDbClient(DBCL_NAME);
    dbClientPtr->execSqlAsync("select * from comments where id " + comp_map.at(comp) + offset +
        " order by id " + order + " limit " + num + ";",
        [=](const drogon::orm::Result &result)
        {
            std::string comments;
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
            callback(resp);
        },
        [=](const drogon::orm::DrogonDbException &e)
        {
            LOG_DEBUG << "error:" << e.base().what() << endl;
            resp->setStatusCode(k400BadRequest);
            callback(resp);
        });
}

void BoardCtrl::get_pfp(const HttpRequestPtr &req,
                        std::function<void(const HttpResponsePtr &)> &&callback,
                        const std::string &username)
{
    LOG_DEBUG << username << endl;

    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    auto dbClientPtr = drogon::app().getFastDbClient(DBCL_NAME);
    dbClientPtr->execSqlAsync(
        "select pfp from users where username=$1",
        [=](const drogon::orm::Result &result)
        {
            if (!result.size())
            {
                resp->setBody("");
            }
            else
            {
                resp->setBody(result.front()["pfp"].as<std::string>());
            }
            callback(resp);
        },
        [=](const drogon::orm::DrogonDbException &e)
        {
            LOG_DEBUG << "error:" << e.base().what() << endl;
            resp->setStatusCode(k404NotFound);
            callback(resp);
        },
        username);
}

void BoardCtrl::edit_profile(const HttpRequestPtr &req,
                             std::function<void(const HttpResponsePtr &)> &&callback)
{
    HttpResponsePtr resp = HttpResponse::newHttpResponse();
    resp->setBody(file_to_string(PAGES_DIR + "edit_profile.html"));
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
    LOG_DEBUG << username << ", pfp sz in B" << pfp.length() << endl;
    auto dbClientPtr = drogon::app().getFastDbClient(DBCL_NAME);
    dbClientPtr->execSqlAsync(
        "update users set pfp = $1 where username=$2",
        [=](const drogon::orm::Result &result)
        {
            LOG_DEBUG << "rows affected: " << result.affectedRows() << endl;
            callback(resp);
        },
        [=](const drogon::orm::DrogonDbException &e)
        {
            LOG_DEBUG << "error:" << e.base().what() << endl;
            resp->setStatusCode(k406NotAcceptable);
            callback(resp);
        },
        pfp, username);
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
    auto dbClientPtr = drogon::app().getFastDbClient(DBCL_NAME);
    dbClientPtr->execSqlAsync("insert into comments (username, date_time, comment) values ($1, $2, $3);",
        [=](const drogon::orm::Result &result)
        {
            resp->setStatusCode(k202Accepted);
            callback(resp);
        },
        [=](const drogon::orm::DrogonDbException &e)
        {
            LOG_DEBUG << "error:" << e.base().what() << endl;
            resp->setStatusCode(k406NotAcceptable);
            callback(resp);
        },
        username, timestamp.str(), comment);
}
