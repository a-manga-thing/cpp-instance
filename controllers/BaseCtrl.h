#pragma once

#include "Types.h"
#include "Manga.h"
#include "Chapter.h"
#include "Globals.h"
#include <fmt/core.h>
#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

using namespace drogon;

using namespace drogon_model::sqlite3;

class BaseCtrl: public RestfulController
{
public:
    
    const std::string dbClientName_{"default"};
    
    orm::DbClientPtr getDbClient() 
    {
        return drogon::app().getDbClient(dbClientName_);
    }
    
    BaseCtrl():RestfulController({}){}  //placeholder

protected:
    
    template<class T>
    std::string getGlobalKey(CJR json)
    {
        return json[T::Cols::_global_id].asString();
    }
    
    template<class T>
    int getId(CJR json)
    {
        return json[T::Cols::_id].asInt();
    }
    
    template <typename V, class T>
    bool getBy(CSR col, V value, T& object)
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
    bool getById(CJR json, T& object)
    {
        auto key = getId<T>(json);
        drogon::orm::Mapper<T> mapper(getDbClient());
        return getBy(T::Cols::_id, key, object);
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

    template <class T>
    void addGlobalId(T& object)
    {
        drogon::orm::Mapper<T> mapper(getDbClient());
        auto id = fmt::format("{}@{}", object.getValueOfId(), globals.instance.url);
        object.setGlobalId(id);
        mapper.update (
            object,
            [object, this](const std::size_t size){ propagate("Create", object); },
            [](const DrogonDbException &e){ LOG_ERROR << e.base().what(); }
        );
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
    
    void addManga(HttpCallback&& callback, CJR json, bool local = false);
    void updateManga(HttpCallback&& callback, CJR json, bool local = false);
    void removeManga(HttpCallback&& callback, CJR json, bool local = false);
    
    void addChapter(HttpCallback&& callback, CJR json, bool local = false);
    void updateChapter(HttpCallback&& callback, CJR json, bool local = false);
    void removeChapter(HttpCallback&& callback, CJR json, bool local = false);
    
    void propagate(CSR action, const Manga& manga);
    void propagate(CSR action, const Chapter& Chapter);
};
