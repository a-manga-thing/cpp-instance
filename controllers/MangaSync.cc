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

void BaseCtrl::addTitle(CSR name, const Manga& manga)
{
    Title title;
    title.setName(name);
    title.setMangaId(manga.getValueOfId());
    drogon::orm::Mapper<Title> mapper(getDbClient());
    mapper.insert(title, [](Title t){}, [](const DrogonDbException& e){});
}

#define LOGEXCEPTCALLBACK \
[](const DrogonDbException& e){ LOG_ERROR << e.base().what(); }

#define ADDRELCALLBACK(Type, Relation) \
[manga, this](Type object) \
{ \
    Relation relation; \
    relation.setMangaId(manga.getValueOfId()); \
    relation.set##Type##Id(object.getValueOfId()); \
    drogon::orm::Mapper< Relation > mapper(getDbClient()); \
    mapper.insert ( \
        relation, \
        [](Relation r){ }, \
        LOGEXCEPTCALLBACK \
    ); \
}

#define ADDRELFUNC(Type, Relation) \
void BaseCtrl::add##Relation(CSR name, const Manga& manga) \
{ \
    auto c = Criteria(Type::Cols::_name,CompareOperator::EQ,name); \
    drogon::orm::Mapper< Type > mapper(getDbClient()); \
    mapper.findOne ( \
        c, \
        ADDRELCALLBACK(Type, Relation), \
        [name, manga, this](const DrogonDbException& e) { \
            Type object; \
            object.setName(name); \
            drogon::orm::Mapper< Type > mapper(getDbClient()); \
            mapper.insert( \
                object, \
                ADDRELCALLBACK(Type, Relation), \
                LOGEXCEPTCALLBACK \
            ); \
        } \
    ); \
}

ADDRELFUNC(Person, Author)
ADDRELFUNC(Person, Artist)
ADDRELFUNC(Tag, MangaTag)

#define ADDRELSFUNC(Type, Array) \
void BaseCtrl::add##Type##s(JSP jsonPtr, const Manga& manga) \
{ \
    const auto& json = *jsonPtr; \
    if (json.isMember(#Array)) { \
        auto& array = json[#Array]; \
        if (array.isArray()) \
            for (auto& itr : array) add##Type(itr.asString(), manga); \
    } \
}

ADDRELSFUNC(Title, titles)
ADDRELSFUNC(Author, authors)
ADDRELSFUNC(Artist, artists)
ADDRELSFUNC(MangaTag, genres)

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
void BaseCtrl::update##Type##s(JSP jsonPtr, const Manga& manga) \
{ \
    remove##Type##s(manga); \
    add##Type##s(jsonPtr, manga); \
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
    
    if (!local && getByGlobalKey(json, manga)) {
        badRequest(callback, "Bad global key");
        return;
    }
    
    manga = Manga(json);
    
    drogon::orm::Mapper<Manga> mapper(getDbClient());
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto jsonPtr = std::make_shared<Json::Value>(json);
    
    mapper.insert (
        manga,
        [callbackPtr, jsonPtr, local, this](Manga manga)
        {
            addTitles(jsonPtr, manga);
            addAuthors(jsonPtr, manga);
            addArtists(jsonPtr, manga);
            addMangaTags(jsonPtr, manga);
            
            if (local) addGlobalId(jsonPtr, manga);
            
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
    auto jsonPtr = std::make_shared<Json::Value>(json);
    
    mapper.deleteBy(
        c,
        [callbackPtr, jsonPtr, manga, local, this](const std::size_t count)
        {
            removeTitles(manga);
            removeAuthors(manga);
            removeArtists(manga);
            removeMangaTags(manga);
            
            if(local) propagate("Delete", "Manga", *jsonPtr);
            
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
            updateTitles(jsonPtr, manga);
            updateAuthors(jsonPtr, manga);
            updateArtists(jsonPtr, manga);
            updateMangaTags(jsonPtr, manga);
            
            if(local) propagate("Modify", "Manga", *jsonPtr);
            
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
