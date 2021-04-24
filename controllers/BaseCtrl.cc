#include "BaseCtrl.h"
#include "Util.h"

bool BaseCtrl::validate(HttpCallback& callback, CJPR jsonPtr)
{
    std::string err;
    
    if(!jsonPtr) {
        badRequest(callback, "No json object is found in the request");
        return false;
    }
    
    if(!doCustomValidations(*jsonPtr, err)) {  //TODO: add json validator
        badRequest(callback, err);  //for all the required fields
        return false;
    }
    
    return true;
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

void BaseCtrl::propagate(CSR action, CSR type, CJR payload)
{
    auto r = HttpRequest::newHttpRequest();
    r->setMethod(drogon::Get);
    r->setPath("/sync/push");
    
    Json::Value json;
    json["action"] = action;
    json["item_type"] = type;
    json["instance"] = globals.instance.url;
    
    globals.forAllFollowers (
        [&](const Instance& itr) {
            json["payload"] = encrypt(itr.url, payload.toStyledString(), itr.publicKey);
            r->setBody(json.toStyledString());
            sendPushReq(r, itr);
        }
    );
}
