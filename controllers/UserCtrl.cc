#include "UserCtrl.h"
#include "Globals.h"
#include "Util.h"
#include "User.h"
#include <fmt/core.h>

void UserCtrl::login(const HttpRequestPtr& req, HttpCallback&& callback)
{
    auto jsonPtr=req->jsonObject();
    if(!validate(callback, jsonPtr)) return;
    
    auto name = (*jsonPtr)["name"].asString();
    auto pass = (*jsonPtr)["pass"].asString();
    auto sessionPtr = req->session();
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    
    drogon::orm::Mapper<User> mapper(getDbClient());
    mapper.findByPrimaryKey (
        name,
        [callbackPtr, sessionPtr, name, pass] (User user)
        {
            HttpStatusCode code;
            Json::Value json;
            
            if (md5(pass+user.getValueOfSalt()) == user.getValueOfPass()) {
                sessionPtr->insert("user", user);
                json["message"] = fmt::format("logged in as {}", name);
                code = k200OK;
            } else {
                json["message"] = fmt::format("wrong password");
                code = k403Forbidden;
            }
            
            auto resp = HttpResponse::newHttpJsonResponse(json);
            resp->setStatusCode(code);
            (*callbackPtr)(resp);
        },
        [callbackPtr](const DrogonDbException &e)
        {
            LOG_ERROR << e.base().what();
            Json::Value ret;
            ret["error"] = "wrong username or database error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            (*callbackPtr)(resp);
        }
    );
}

bool UserCtrl::isLogged(const HttpRequestPtr& req, HttpCallback& callback)
{
    bool ret = req->session()->find("user");
    if (!ret) badRequest(callback, "", k403Forbidden);
    return ret;
}

#define GENMETHOD(OP, Type) \
void UserCtrl::OP##Type(const HttpRequestPtr& req, HttpCallback&& callback) \
{ \
    auto jsonPtr=req->jsonObject(); \
    if(!isLogged(req, callback)) return; \
    if(!validate(callback, jsonPtr)) return; \
    BaseCtrl::OP##Type(std::move(callback), *jsonPtr, true); \
}

GENMETHOD(add, Manga)
GENMETHOD(remove, Manga)
GENMETHOD(update, Manga)
GENMETHOD(add, Chapter)
GENMETHOD(remove, Chapter)
GENMETHOD(update, Chapter)
