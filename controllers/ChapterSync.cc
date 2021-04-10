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

static void sendPushReq(const HttpRequestPtr& req, Instance instance)
{
    auto client = HttpClient::newHttpClient(instance.url);
    client->sendRequest(
        req,
        [instance]
        (ReqResult result, const HttpResponsePtr& response) {
            if (result != ReqResult::Ok)
                globals.removeFollower(instance.url);
        },
        30.0  //TODO: use config values
    );
}

void BaseCtrl::propagate(CSR action, const Chapter& chapter)
{
    auto r = HttpRequest::newHttpRequest();
    r->setMethod(drogon::Get);
    r->setPath(fmt::format("/sync/accept?address={}", globals.instance.url));
    
    auto chapterJson = chapter.toJson();
    Json::Value json;
    json["action"] = action;
    json["item_type"] = "Chapter";
    json["instance"] = globals.instance.url;
    
    globals.forAllFollowers (
        [&](const Instance& itr) {
            json["payload"] = encrypt(itr.url, chapterJson.toStyledString(), itr.publicKey);
            r->setBody(json.toStyledString());
            sendPushReq(r, itr);
        }
    );
}
