#include "BaseCtrl.h"
#include "Chapter.h"
#include "Title.h"
#include "Person.h"
#include "Author.h"
#include "Artist.h"
#include "MangaTag.h"
#include "Tag.h"
#include "Util.h"
#include "Globals.h"
#include <fmt/core.h>
#include <sstream>
#include <string>

template <class T>
static T getOrCreate(orm::DbClientPtr dbClientPtr, CSR name)
{
    auto c = Criteria(T::Cols::_name,CompareOperator::EQ,name);
    drogon::orm::Mapper<T> mapper(dbClientPtr);
    try {
        return mapper.findOne(c);
    } catch (const DrogonDbException &e) {
        T object;
        object.setName(name);
        mapper.insert(object);
        return object;
    }
}

void BaseCtrl::addTitle(CSR name, const Manga& manga)
{
    Title title;
    title.setName(name);
    title.setMangaId(manga.getValueOfId());
    drogon::orm::Mapper<Title> mapper(getDbClient());
    mapper.insertFuture(title);
}

#define ADDRELFUNC(Type, Relation) \
void BaseCtrl::add##Relation(CSR name, const Manga& manga) \
{ \
    Type object = getOrCreate< Type >(getDbClient(), name); \
    Relation relation; \
    relation.setMangaId(manga.getValueOfId()); \
    relation.set##Type##Id(object.getValueOfId()); \
    drogon::orm::Mapper< Relation > mapper(getDbClient()); \
    mapper.insertFuture(relation); \
}

ADDRELFUNC(Person, Author)
ADDRELFUNC(Person, Artist)
ADDRELFUNC(Tag, MangaTag)

#define ADDRELSFUNC(Type, Array) \
void BaseCtrl::add##Type##s(CJR json, const Manga& manga) \
{ \
    if (json.isMember(#Array)) { \
        auto& array = json[#Array]; \
        if (array.isArray()) \
            for (auto& itr : array) add##Type(itr.asString(), manga); \
    } \
}

ADDRELSFUNC(Title, titles)
ADDRELSFUNC(Author, autors)
ADDRELSFUNC(Artist, artists)
ADDRELSFUNC(MangaTag, tags)

#define REMOVERELSFUNC(Type) \
void BaseCtrl::remove##Type##s(const Manga& manga) \
{ \
    removeRelation< Type >(manga); \
}

REMOVERELSFUNC(Title)
REMOVERELSFUNC(Author)
REMOVERELSFUNC(Artist)
REMOVERELSFUNC(MangaTag)

#define UPDATERELSFUNC(Type) \
void BaseCtrl::update##Type##s(CJR json, const Manga& manga) \
{ \
    remove##Type##s(manga); \
    add##Type##s(json, manga); \
}

UPDATERELSFUNC(Title)
UPDATERELSFUNC(Author)
UPDATERELSFUNC(Artist)
UPDATERELSFUNC(MangaTag)

void BaseCtrl::addManga(HttpCallback&& callback, CJR json, bool local)
{
    Manga manga;
    std::string err;
    
    if(!Manga::validateJsonForCreation(json, err)) {
        badRequest(callback, err);
        return;
    }
    
    if (
        (!local && getByGlobalKey(json, manga))
        || (local && getById(json, manga))
    ) {
        badRequest(callback, "Bad key");
        return;
    }
    
    drogon::orm::Mapper<Manga> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto jsonPtr = std::make_shared<Json::Value>(json);
    
    mapper.insert (
        Manga(json),
        [callbackPtr, jsonPtr, local, this](Manga manga)
        {
            addTitles(*jsonPtr, manga);
            addAuthors(*jsonPtr, manga);
            addArtists(*jsonPtr, manga);
            addMangaTags(*jsonPtr, manga);
            
            if (local) addGlobalId(manga);
            
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

void BaseCtrl::removeManga(HttpCallback&& callback, CJR json, bool local)
{
    Manga manga;
    
    if (
        (!local && getByGlobalKey(json, manga))
        || (local && getById(json, manga))
    ) {
        badRequest(callback, "Bad key");
        return;
    }
    
    drogon::orm::Mapper<Manga> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto c = Criteria(Manga::Cols::_id,CompareOperator::EQ, manga.getValueOfId());
    
    mapper.deleteBy(
        c,
        [callbackPtr, manga, local, this](const std::size_t count)
        {
            removeTitles(manga);
            removeAuthors(manga);
            removeArtists(manga);
            removeMangaTags(manga);
            
            if(local) propagate("Delete", manga);
            
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

void BaseCtrl::updateManga(HttpCallback&& callback, CJR json, bool local)
{
    Manga manga;
    std::string err;
    
    if(!Manga::validateJsonForUpdate(json, err)) {
        badRequest(callback, err);
        return;
    }
    
    if (
        (!local && !getByGlobalKey(json, manga))
        || (local && !getById(json, manga))
    ) {
        badRequest(callback, "Bad key");
        return;
    }
    
    try {
        manga.updateByJson(json);
    } catch (const Json::Exception &e) {
        badRequest(callback, "Field type error");
        return;
    }
    
    drogon::orm::Mapper<Manga> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto jsonPtr = std::make_shared<Json::Value>(json);
    
    mapper.update(
        manga,
        [callbackPtr, jsonPtr, manga, local, this](const std::size_t count)
        {
            updateTitles(*jsonPtr, manga);
            updateAuthors(*jsonPtr, manga);
            updateArtists(*jsonPtr, manga);
            updateMangaTags(*jsonPtr, manga);
            
            if(local) propagate("Update", manga);
            
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

void BaseCtrl::propagate(CSR action, const Manga& manga)
{
    auto r = HttpRequest::newHttpRequest();
    r->setMethod(drogon::Get);
    r->setPath(fmt::format("/sync/accept?address={}", globals.instance.url));
    
    auto mangaJson = mangaToJson(getDbClient(), manga);
    Json::Value json;
    json["action"] = action;
    json["item_type"] = "Manga";
    json["instance"] = globals.instance.url;
    
    globals.forAllFollowers (
        [&](const Instance& itr) {
            json["payload"] = encrypt(itr.url, mangaJson.toStyledString(), itr.publicKey);
            r->setBody(json.toStyledString());
            sendPushReq(r, itr);
        }
    );
}
