#pragma once

#include "Types.h"
#include "User.h"
#include "BaseCtrl.h"
#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

using namespace drogon;

using namespace drogon_model::sqlite3;

class UserCtrl : public HttpController<UserCtrl>, public BaseCtrl
{
public:
    
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(UserCtrl::login,"/mod/login",Post,Options);
    ADD_METHOD_TO(UserCtrl::addManga,"/mod/manga",Post,Options);
    ADD_METHOD_TO(UserCtrl::removeManga,"/mod/manga",Delete,Options);
    ADD_METHOD_TO(UserCtrl::updateManga,"/mod/manga",Put,Options);
    ADD_METHOD_TO(UserCtrl::addChapter,"/mod/chapter",Post,Options);
    ADD_METHOD_TO(UserCtrl::removeChapter,"/mod/chapter",Delete,Options);
    ADD_METHOD_TO(UserCtrl::updateChapter,"/mod/chapter",Put,Options);
    METHOD_LIST_END
    
    void login(const HttpRequestPtr& req, HttpCallback&& callback);
    void addManga(const HttpRequestPtr& req, HttpCallback&& callback);
    void removeManga(const HttpRequestPtr& req, HttpCallback&& callback);
    void updateManga(const HttpRequestPtr& req, HttpCallback&& callback);
    void addChapter(const HttpRequestPtr& req, HttpCallback&& callback);
    void removeChapter(const HttpRequestPtr& req, HttpCallback&& callback);
    void updateChapter(const HttpRequestPtr& req, HttpCallback&& callback);
    
private:
    
    bool isLogged(const HttpRequestPtr& req, HttpCallback& callback);
    
};
