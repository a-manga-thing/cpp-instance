#pragma once

#include "Types.h"
#include "Manga.h"
#include "MangaCtrlBase.h"
#include <drogon/HttpController.h>

using namespace drogon;

using namespace drogon_model::sqlite3;

class MangaCtrl: public drogon::HttpController<MangaCtrl>, public MangaCtrlBase
{
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
    
    std::string getGlobalKey(CJPR jsonPtr)
    {
        return (*jsonPtr)[Manga::Cols::_global_id].asString();
    }
    
    bool getByGlobalKey(CJPR jsonPtr, Manga& manga)
    {
        auto key = getGlobalKey(jsonPtr);
        drogon::orm::Mapper<Manga> mapper(getDbClient());
        return getBy(Manga::Cols::_global_id, key, manga);
    }
    
    template <class T>
    void removeRelation(const Manga& manga)
    {
        auto c = Criteria(T::Cols::_manga_id,CompareOperator::EQ,manga.getValueOfId());
        drogon::orm::Mapper<T> mapper(getDbClient());
        //mapper.deleteFutureBy(c);
    }
    
    bool validate(CJPR jsonPtr, HttpCallback& callback);
    
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
    
public:
    
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MangaCtrl::add,"/sync/manga",Post,Options);
    ADD_METHOD_TO(MangaCtrl::update,"/sync/manga",Put,Options);
    ADD_METHOD_TO(MangaCtrl::remove,"/sync/manga",Delete,Options);
    ADD_METHOD_TO(MangaCtrl::getSearch,"/manga/search?title={1}&authors={2}&artists={3}&tags={4}",Get,Options);
    ADD_METHOD_TO(MangaCtrl::getFromId,"/manga/from_id?id={1}",Get,Options);
    ADD_METHOD_TO(MangaCtrl::getChapter,"/manga/get_chapter?mangaid={1}&ordinal={2}",Get,Options);
    ADD_METHOD_TO(MangaCtrl::getThumbnail,"/manga/thumbnail?mangaid={1}",Get,Options);
    METHOD_LIST_END
    
    void add(const HttpRequestPtr& req, HttpCallback&& callback);
    void update(const HttpRequestPtr& req, HttpCallback&& callback);
    void remove(const HttpRequestPtr& req, HttpCallback&& callback);
    
    void getSearch(const HttpRequestPtr& req, HttpCallback&& callback, CSR title, CSR authorsCSV, CSR artistsCSV, CSR tagsCSV);
    void getFromId(const HttpRequestPtr& req, HttpCallback&& callback, long id);
    void getChapter(const HttpRequestPtr& req, HttpCallback&& callback, long mangaid, int ordinal);
    void getThumbnail(const HttpRequestPtr& req, HttpCallback&& callback, long mangaid);
    
    MangaCtrl();
};
