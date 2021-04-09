#pragma once

#include "Types.h"
#include "Manga.h"
#include "Chapter.h"
#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

using namespace drogon;

using namespace drogon_model::sqlite3;

class SyncCtrl: public HttpController<SyncCtrl>, public RestfulController
{
public:
    
    const std::string dbClientName_{"default"};
    
    orm::DbClientPtr getDbClient() 
    {
        return drogon::app().getDbClient(dbClientName_);
    }
    
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(SyncCtrl::subscribe,"/sync/subscribe?address={1}",Get,Options);
    ADD_METHOD_TO(SyncCtrl::accept,"/sync/accept?address={1}",Get,Options);
    ADD_METHOD_TO(SyncCtrl::push,"/sync/push",Post,Options);
    METHOD_LIST_END
    
    void subscribe(const HttpRequestPtr& req, HttpCallback&& callback, CSR id);
    void accept(const HttpRequestPtr& req, HttpCallback&& callback, CSR id);
    void push(const HttpRequestPtr& req, HttpCallback&& callback);
    
    SyncCtrl():RestfulController({}){}  //placeholder

private:
    
    template<class T>
    std::string getGlobalKey(CJR json)
    {
        return json[T::Cols::_global_id].asString();
    }
    
    template <class T>
    bool getBy(CSR col, CSR value, T& object)
    {
        drogon::orm::Mapper<T> mapper(getDbClient());
        auto c = Criteria(col,CompareOperator::EQ, value);
        try {
            object = mapper.findOne(c);
            return true;
        } catch (const DrogonDbException &e) {
            return false;
        }
    }
    
    template<class T>
    bool getByGlobalKey(CJR json, T& object)
    {
        auto key = getGlobalKey<T>(json);
        drogon::orm::Mapper<T> mapper(getDbClient());
        return getBy(T::Cols::_global_id, key, object);
    }
    
    template <class T>
    void removeRelation(const Manga& manga)
    {
        auto c = Criteria(T::Cols::_manga_id,CompareOperator::EQ,manga.getValueOfId());
        drogon::orm::Mapper<T> mapper(getDbClient());
        //mapper.deleteFutureBy(c);
    }
    
    bool validate(HttpCallback& callback, CJPR jsonPtr);
    
    void addTitle(CSR title, const Manga& manga);
    void addAuthor(CSR author, const Manga& manga);
    void addArtist(CSR artist, const Manga& manga);
    void addMangaTag(CSR tag, const Manga& manga);
    
    void addTitles(CJR json, const Manga& manga);
    void addAuthors(CJR json, const Manga& manga);
    void addArtists(CJR json, const Manga& manga);
    void addMangaTags(CJR json, const Manga& manga);
    
    void removeTitles(const Manga& manga);
    void removeAuthors(const Manga& manga);
    void removeArtists(const Manga& manga);
    void removeMangaTags(const Manga& manga);
    
    void updateTitles(CJR json, const Manga& manga);
    void updateAuthors(CJR json, const Manga& manga);
    void updateArtists(CJR json, const Manga& manga);
    void updateMangaTags(CJR json, const Manga& manga);
    
    void addManga(HttpCallback&& callback, CJR json);
    void updateManga(HttpCallback&& callback, CJR json);
    void removeManga(HttpCallback&& callback, CJR json);
    
    void addChapter(HttpCallback&& callback, CJR json);
    void updateChapter(HttpCallback&& callback, CJR json);
    void removeChapter(HttpCallback&& callback, CJR json);
};
