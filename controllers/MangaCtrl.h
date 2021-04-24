#pragma once

#include "Types.h"
#include "Manga.h"
#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

using namespace drogon;

using namespace drogon_model::sqlite3;

class MangaCtrl: public HttpController<MangaCtrl>, public RestfulController
{
public:
    
    const std::string dbClientName_{"default"};
    
    orm::DbClientPtr getDbClient() 
    {
        return drogon::app().getDbClient(dbClientName_);
    }
    
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MangaCtrl::search,"/manga/search?title={1}&authors={2}&artists={3}&tags={4}",Get,Options);
    ADD_METHOD_TO(MangaCtrl::fromId,"/manga/from_id?id={1}",Get,Options);
    ADD_METHOD_TO(MangaCtrl::chapter,"/manga/get_chapter?mangaid={1}&ordinal={2}",Get,Options);
    ADD_METHOD_TO(MangaCtrl::thumbnail,"/manga/thumbnail?mangaid={1}",Get,Options);
    METHOD_LIST_END
    
    void search(const HttpRequestPtr& req, HttpCallback&& callback, CSR title, CSR authorsCSV, CSR artistsCSV, CSR tagsCSV);
    void fromId(const HttpRequestPtr& req, HttpCallback&& callback, long id);
    void chapter(const HttpRequestPtr& req, HttpCallback&& callback, long mangaid, int ordinal);
    void thumbnail(const HttpRequestPtr& req, HttpCallback&& callback, long mangaid);
    
    MangaCtrl();
};
