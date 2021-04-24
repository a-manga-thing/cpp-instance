#include "BaseCtrl.h"
#include "Chapter.h"
#include "Util.h"
#include "Globals.h"
#include <fmt/core.h>
#include <sstream>
#include <string>

void BaseCtrl::addChapter(HttpCallback&& callback, CJR json, bool local)
{  //TODO: map global_manga_id to local manga_id
    Chapter chapter;
    std::string err;
    
    if(!Chapter::validateJsonForCreation(json, err)) {
        badRequest(callback, err);
        return;
    }
    
    if(getByGlobalKey(json, chapter)) {
        badRequest(callback, "Bad global key");
        return;
    }
    
    drogon::orm::Mapper<Chapter> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto jsonPtr = std::make_shared<Json::Value>(json);
    
    mapper.insert(
        Chapter(json),
        [callbackPtr, jsonPtr, local, this](Chapter chapter)
        {
            if(local) addGlobalId(jsonPtr, chapter);
            
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k202Accepted);
            (*callbackPtr)(resp);
        },
        [callbackPtr](const DrogonDbException &e)
        {
            LOG_ERROR << e.base().what();
            Json::Value ret;
            ret["error"] = "database error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            (*callbackPtr)(resp);
        }
    );
}

void BaseCtrl::removeChapter(HttpCallback&& callback, CJR json, bool local)
{
    Chapter chapter;
    
    if(!getByGlobalKey(json, chapter)) {
        badRequest(callback, "Bad global key");
        return;
    }
    
    drogon::orm::Mapper<Chapter> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto c = Criteria(Chapter::Cols::_id,CompareOperator::EQ, chapter.getValueOfId());
    auto jsonPtr = std::make_shared<Json::Value>(json);
    
    mapper.deleteBy(
        c,
        [callbackPtr, jsonPtr, local, chapter, this](const std::size_t count)
        {
            if (local) propagate("Delete", "Chapter", *jsonPtr);
            
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k202Accepted);
            (*callbackPtr)(resp);
        },
        [callbackPtr](const DrogonDbException &e)
        {
            LOG_ERROR << e.base().what();
            Json::Value ret;
            ret["error"] = "database error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            (*callbackPtr)(resp);
        }
    );
};

void BaseCtrl::updateChapter(HttpCallback&& callback, CJR json, bool local)
{  //TODO: mask manga_id and manga_global_id
    Chapter chapter;
    std::string err;
    
    if(!Chapter::validateJsonForUpdate(json, err)) {
        badRequest(callback, err);
        return;
    }
    
    if(!getByGlobalKey(json, chapter)) {
        badRequest(callback, "Bad global key");
        return;
    }
    
    try {
        chapter.updateByJson(json);
    } catch (const Json::Exception &e) {
        badRequest(callback, "Field type error");
        return;
    }
    
    drogon::orm::Mapper<Chapter> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto jsonPtr = std::make_shared<Json::Value>(json);
    
    mapper.update(
        chapter,
        [callbackPtr, jsonPtr, local, chapter, this](const std::size_t count)
        {
            if (local) propagate("Modify", "Chapter", *jsonPtr);
            
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k202Accepted);
            (*callbackPtr)(resp);
        },
        [callbackPtr](const DrogonDbException &e)
        {
            LOG_ERROR << e.base().what();
            Json::Value ret;
            ret["error"] = "database error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            (*callbackPtr)(resp);
        }
    );
}
