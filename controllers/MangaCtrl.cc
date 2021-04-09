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

#define MAKETEMPLATE(table, rel) " manga.id IN (SELECT "#rel".manga_id FROM "#rel" JOIN "#table" ON "#table".id = "#rel"."#table"_id WHERE "#table".name IN ({}) GROUP BY "#rel".manga_id HAVING COUNT(DISTINCT "#table".name) = {} )"
#define MAKEEXTEMPLATE(table, rel) " manga.id NOT IN (SELECT "#rel".manga_id FROM "#rel" JOIN "#table" ON "#table".id = "#rel"."#table"_id WHERE "#table".name IN ({}) GROUP BY "#rel".manga_id HAVING COUNT(DISTINCT "#table".name) = {} )"

static void addWith (
    bool& mFilter,
    std::stringstream& ss,
    const std::string& fmtstr,
    const std::vector<std::string>& vec,
    std::size_t minSize
) {
    static constexpr auto andQuery = " AND";
    
    if (vec.size()) {
        if (mFilter) ss << andQuery;
        ss << fmt::format(fmtstr, joinVec(vec), minSize);
        mFilter = true;
    }
}

static std::string searchQuery (
    CSR title,
    const std::vector<std::string> authors,
    const std::vector<std::string> artists,
    const std::vector<std::string> tags,
    const std::vector<std::string> tagsEx
) {
    static constexpr auto selectQuery = "SELECT manga.*";
    static constexpr auto fromQuery = " FROM manga";
    static constexpr auto joinQuery = " INNER JOIN title ON manga.id = title.manga_id";
    static constexpr auto whereQuery = " WHERE";
    static constexpr auto titleQueryTemplate = " title.name LIKE '%{}%'";
    static constexpr auto authorQueryTemplate = MAKETEMPLATE(person,author);
    static constexpr auto artistQueryTemplate = MAKETEMPLATE(person,artist);
    static constexpr auto tagQueryTemplate = MAKETEMPLATE(tag,manga_tag);
    static constexpr auto tagExQueryTemplate = MAKEEXTEMPLATE(tag,manga_tag);
    
    bool mFilter{false};
    std::stringstream ss;
    ss << selectQuery << fromQuery;
    
    if (title.size()) {
        ss << joinQuery << whereQuery << fmt::format(titleQueryTemplate, title);
        mFilter = true;
    } else if (authors.size() || artists.size() || tags.size() || tagsEx.size()) {
        ss << whereQuery;
    } else goto re;
    
    addWith(mFilter, ss, authorQueryTemplate, authors, authors.size());
    addWith(mFilter, ss, artistQueryTemplate, artists, artists.size());
    addWith(mFilter, ss, tagQueryTemplate, tags, tags.size());
    addWith(mFilter, ss, tagExQueryTemplate, tagsEx, 1);
    
re: ss << ";";
    return ss.str();
}

#define GENERICQUERY(Type) "SELECT "#Type".name FROM "#Type" WHERE ("#Type".manga_id = {});"

#define GETSIMPLEREL(Type) { \
auto query = fmt::format(GENERICQUERY(Type), manga.getValueOfId()); \
auto result = dbClientPtr->execSqlSync(query); \
for (auto& row : result) ret[ #Type"s" ].append(row.begin()->as<std::string>()); \
}

#define COMPLEXQUERY(Type, Relation, IdField) "SELECT "#Type".name FROM "#Type" INNER JOIN "#Relation" ON "#Type".id = "#Relation"."#IdField" WHERE ("#Relation".manga_id = {});"

#define GETCOMPLEXREL(Type, Relation, IdField, Label) { \
auto query = fmt::format(COMPLEXQUERY(Type, Relation, IdField), manga.getValueOfId()); \
auto result = dbClientPtr->execSqlSync(query); \
for (auto& row : result) ret[ Label ].append(row.begin()->as<std::string>()); \
}

static Json::Value mangaToJson(orm::DbClientPtr dbClientPtr, const Manga& manga)
{
    auto ret = manga.toJson();
    
    GETSIMPLEREL(title)
    GETCOMPLEXREL(person, author, person_id, "authors")
    GETCOMPLEXREL(person, artist, person_id, "artists")
    GETCOMPLEXREL(tag, manga_tag, tag_id, "tags")
    
    return ret;
}

void MangaCtrl::getSearch (
    const HttpRequestPtr& req,
    HttpCallback&& callback,
    CSR title,
    CSR authorsCSV,
    CSR artistsCSV,
    CSR tagsCSV
) {
    std::vector<std::string> tags, tagsEx;
    auto authors = splitCSV(authorsCSV);
    auto artists = splitCSV(artistsCSV);
    splitCSV(tagsCSV, tags, tagsEx);
    
    auto dbClientPtr = getDbClient();
    
    auto result = dbClientPtr->execSqlSync(
        searchQuery(title, authors, artists, tags, tagsEx)
    );
    
    Json::Value ret;
    for (auto& itr : result) ret.append(mangaToJson(dbClientPtr,Manga(itr)));
    callback(HttpResponse::newHttpJsonResponse(ret));
}

void MangaCtrl::getFromId(const HttpRequestPtr& req, HttpCallback&& callback, long id)
{
    static constexpr auto query = "SELECT * FROM manga WHERE id = {};";
    
    auto dbClientPtr = getDbClient();
    auto result = dbClientPtr->execSqlSync(fmt::format(query, id));
    
    if (result.size()) {
        auto json = mangaToJson(dbClientPtr,Manga(*result.begin()));
        callback(HttpResponse::newHttpJsonResponse(json));
    } else {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        callback(resp);
    }
    
}

void MangaCtrl::getChapter(const HttpRequestPtr& req, HttpCallback&& callback, long mangaid, int ordinal)
{
    auto dbClientPtr = getDbClient();
    auto callbackPtr = std::make_shared<HttpCallback>(std::move(callback));
    auto mc = Criteria(Chapter::Cols::_manga_id,CompareOperator::EQ,mangaid);
    auto oc = Criteria(Chapter::Cols::_ordinal,CompareOperator::EQ,ordinal);
    
    drogon::orm::Mapper<Chapter> mapper(dbClientPtr);
    mapper.findOne(
        mc && oc,
        [req, callbackPtr, this](Chapter r) {
            (*callbackPtr)(HttpResponse::newHttpJsonResponse(makeJson(req, r)));
        },
        [callbackPtr](const DrogonDbException &e) {
            LOG_ERROR<<e.base().what();
            Json::Value ret;
            ret["error"] = "database error";
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k500InternalServerError);
            (*callbackPtr)(resp);
        });
}

void MangaCtrl::getThumbnail(const HttpRequestPtr& req, HttpCallback&& callback, long mangaid)
{
    //manga = ...
    //if (!fs::file_exists(...)) make_thumbnail(...);
    //resp = ...
}

MangaCtrl::MangaCtrl()
: RestfulController({
    "id",
    "type",
    "country_of_origin",
    "publication_status",
    "scanlation_status",
    "mal_id",
    "anilist_id",
    "mangaupdates_id",
    "global_id",
    "update"})
{
    enableMasquerading({
        "id", // the alias for the id column.
        "type", // the alias for the type column.
        "country_of_origin", // the alias for the country_of_origin column.
        "publication_status", // the alias for the publication_status column.
        "scanlation_status", // the alias for the scanlation_status column.
        "mal_id", // the alias for the mal_id column.
        "anilist_id", // the alias for the anilist_id column.
        "mangaupdates_id", // the alias for the mangaupdates_id column.
        "global_id", // the alias for the global_id column.
        "update"  // the alias for the update column.
    });
}
