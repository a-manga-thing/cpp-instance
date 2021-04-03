#include "MangaCtrl.h"
#include "Chapter.h"
#include "Title.h"
#include "Person.h"
#include "Author.h"
#include "Artist.h"
#include "MangaTag.h"
#include "Tag.h"
#include "Util.h"
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

void MangaCtrl::addTitle(CSR name, const Manga& manga)
{
    Title title;
    title.setName(name);
    title.setMangaId(manga.getValueOfId());
    drogon::orm::Mapper<Title> mapper(getDbClient());
    mapper.insertFuture(title);
}

#define ADDRELFUNC(Type, Relation) \
void MangaCtrl::add##Relation(CSR name, const Manga& manga) \
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
void MangaCtrl::add##Type##s(CJR json, const Manga& manga) \
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
void MangaCtrl::remove##Type##s(const Manga& manga) \
{ \
    removeRelation< Type >(manga); \
}

REMOVERELSFUNC(Title)
REMOVERELSFUNC(Author)
REMOVERELSFUNC(Artist)
REMOVERELSFUNC(MangaTag)

#define UPDATERELSFUNC(Type) \
void MangaCtrl::update##Type##s(CJR json, const Manga& manga) \
{ \
    remove##Type##s(manga); \
    add##Type##s(json, manga); \
}

UPDATERELSFUNC(Title)
UPDATERELSFUNC(Author)
UPDATERELSFUNC(Artist)
UPDATERELSFUNC(MangaTag)

static void badRequest(HttpCallback& callback, CSR err)
{
    Json::Value ret;
    ret["error"]=err;
    auto resp= HttpResponse::newHttpJsonResponse(ret);
    resp->setStatusCode(k400BadRequest);
    callback(resp);
}

bool MangaCtrl::validate(CJPR jsonPtr, HttpCallback& callback)
{
    std::string err;
    
    if(!jsonPtr) {
        badRequest(callback, "No json object is found in the request");
        return false;
    }
    
    if(!doCustomValidations(*jsonPtr, err)) {  //TODO: add json validator
        badRequest(callback, err);  //for globalKey
        return false;
    }
    
    return true;
}

void MangaCtrl::add(const HttpRequestPtr& req, HttpCallback&& callback)
{
    Manga manga;
    std::string err;
    auto jsonPtr=req->jsonObject();
    if(!validate(jsonPtr, callback)) return;
    
    if(!Manga::validateMasqueradedJsonForCreation(*jsonPtr, masqueradingVector(), err)) {
        badRequest(callback, err);
        return;
    }
    
    if(getByGlobalKey(jsonPtr, manga)) {
        badRequest(callback, "Bad global key");
        return;
    }
    
    drogon::orm::Mapper<Manga> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    
    mapper.insert(
        Manga(*jsonPtr),
        [callbackPtr, jsonPtr, this](Manga manga)
        {
            addTitles(*jsonPtr, manga);
            addAuthors(*jsonPtr, manga);
            addArtists(*jsonPtr, manga);
            addMangaTags(*jsonPtr, manga);
            
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

void MangaCtrl::remove(const HttpRequestPtr& req, HttpCallback&& callback)
{
    Manga manga;
    auto jsonPtr=req->jsonObject();
    if(!validate(jsonPtr, callback)) return;
    
    if(!getByGlobalKey(jsonPtr, manga)) {
        badRequest(callback, "Bad global key");
        return;
    }
    
    drogon::orm::Mapper<Manga> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto c = Criteria(Manga::Cols::_id,CompareOperator::EQ, manga.getValueOfId());
    
    mapper.deleteBy(
        c,
        [callbackPtr, manga, this](const std::size_t count)
        {
            removeTitles(manga);
            removeAuthors(manga);
            removeArtists(manga);
            removeMangaTags(manga);
            
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

void MangaCtrl::update(const HttpRequestPtr& req, HttpCallback&& callback)
{
    Manga manga;
    std::string err;
    auto jsonPtr=req->jsonObject();
    if(!validate(jsonPtr, callback)) return;
    
    if(!Manga::validateMasqueradedJsonForUpdate(*jsonPtr, masqueradingVector(), err)) {
        badRequest(callback, err);
        return;
    }
    
    if(!getByGlobalKey(jsonPtr, manga)) {
        badRequest(callback, "Bad global key");
        return;
    }
    
    try {
        manga.updateByMasqueradedJson(*jsonPtr, masqueradingVector());
    } catch (const Json::Exception &e) {
        badRequest(callback, "Field type error");
        return;
    }
    
    drogon::orm::Mapper<Manga> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    
    mapper.update(
        manga,
        [callbackPtr, jsonPtr, manga, this](const std::size_t count)
        {
            updateTitles(*jsonPtr, manga);
            updateAuthors(*jsonPtr, manga);
            updateArtists(*jsonPtr, manga);
            updateMangaTags(*jsonPtr, manga);
            
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
