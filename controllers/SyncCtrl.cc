#include "SyncCtrl.h"
#include "Globals.h"
#include "Util.h"
#include <fmt/core.h>

static std::pair<HttpStatusCode, Json::Value> genResponseData (
    ReqResult result,
    const HttpResponsePtr& response,
    CSR id
) {
    Json::Value json;
    HttpStatusCode code;
    
    switch (result) {
        case ReqResult::Ok: {
                auto jsonPtr = response->jsonObject();
                code = k201Created;
                if (jsonPtr && jsonPtr->isMember("public_key"))
                    globals.addFollower((*jsonPtr)["public_key"].asString(), id);
                json["message"] = "OK";
            } break;
        case ReqResult::BadResponse:
            code = k403Forbidden;
            json["message"] = "The instance did not accept our subscription";
            break;
        default:
            code = k400BadRequest;
            json["message"] = "Could not connect to the instance";
            break;
    }
    
    return {code, json};
}

void SyncCtrl::subscribe(const HttpRequestPtr& req, HttpCallback&& callback, CSR id)
{
    auto client = HttpClient::newHttpClient("http://"+id);
    auto r = HttpRequest::newHttpRequest();
    r->setMethod(drogon::Get);
    r->setPath(fmt::format("/sync/accept?address={}", globals.instance.url));
    
    client->sendRequest(
        r,
        [callback = std::move(callback), id, this]
        (ReqResult result, const HttpResponsePtr& response) {
            auto data = genResponseData(result, response, id);
            auto resp = HttpResponse::newHttpJsonResponse(data.second);
            resp->setStatusCode(data.first);
            callback(resp);
        },
        30.0  //TODO: use config values
    );
}

void SyncCtrl::accept(const HttpRequestPtr& req, HttpCallback&& callback, CSR id)
{
    Json::Value json;
    HttpStatusCode code;
    
    if (globals.inTryFollow(id)) {
        code = k200OK;
        auto instance = globals.addFollowing(id);
        json["public_key"] = instance.publicKey;
    } else {
        code = k403Forbidden;
        json["message"] = "Not expecting subscription from this address";
    }
    
    auto resp = HttpResponse::newHttpJsonResponse(json);
    resp->setStatusCode(code);
    callback(resp);
}

void SyncCtrl::push(const HttpRequestPtr& req, HttpCallback&& callback)
{
    auto jsonPtr=req->jsonObject();
    if(!validate(callback, jsonPtr)) return;
    
    auto action = (*jsonPtr)["action"].asString();
    auto type = (*jsonPtr)["item_type"].asString();
    auto instance = (*jsonPtr)["instance"].asString();
    
    if (!globals.inFollowing(instance)) {
        badRequest(callback, "Not subscribed", k403Forbidden);
        return;
    }
    
    auto payload = decrypt(
        (*jsonPtr)["payload"].asString()
        , globals.getFollowing(instance).privateKey
        , globals.getFollowing(instance).pass
    );
    
    if (type == "Manga") {
        if (action == "Create") addManga(std::move(callback), payload);
        else if (action == "Modify") updateManga(std::move(callback), payload);
        else if (action == "Delete") removeManga(std::move(callback), payload);
        else badRequest(callback, "unknown action");
    }
    
    else if (type == "Chapter") {
        if (action == "Create") addChapter(std::move(callback), payload);
        else if (action == "Modify") updateChapter(std::move(callback), payload);
        else if (action == "Delete") removeChapter(std::move(callback), payload);
        else badRequest(callback, "unknown action");
    }
    
    else badRequest(callback, "unknown item_type");
}
