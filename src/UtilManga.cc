#include "Util.h"
#include "Manga.h"
#include <fmt/core.h>

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

Json::Value mangaToJson (
    drogon::orm::DbClientPtr dbClientPtr,
    const drogon_model::sqlite3::Manga& manga
) {
    auto ret = manga.toJson();
    
    GETSIMPLEREL(title)
    GETCOMPLEXREL(person, author, person_id, "authors")
    GETCOMPLEXREL(person, artist, person_id, "artists")
    GETCOMPLEXREL(tag, manga_tag, tag_id, "tags")
    
    return ret;
}
