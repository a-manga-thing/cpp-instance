#include "SyncCtrl.h"
#include "Chapter.h"
#include "Util.h"
#include <fmt/core.h>
#include <sstream>
#include <string>

void SyncCtrl::addChapter(HttpCallback&& callback, CJR json)
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
    
    mapper.insert(
        Chapter(json),
        [callbackPtr, this](Chapter chapter)
        {
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

void SyncCtrl::removeChapter(HttpCallback&& callback, CJR json)
{
    Chapter chapter;
    
    if(!getByGlobalKey(json, chapter)) {
        badRequest(callback, "Bad global key");
        return;
    }
    
    drogon::orm::Mapper<Chapter> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto c = Criteria(Chapter::Cols::_id,CompareOperator::EQ, chapter.getValueOfId());
    
    mapper.deleteBy(
        c,
        [callbackPtr, chapter, this](const std::size_t count)
        {
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

void SyncCtrl::updateChapter(HttpCallback&& callback, CJR json)
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
    
    mapper.update(
        chapter,
        [callbackPtr, chapter, this](const std::size_t count)
        {
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
