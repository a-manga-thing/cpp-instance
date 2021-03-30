/**
 *
 *  Artist.cc
 *  DO NOT EDIT. This file is generated by drogon_ctl
 *
 */

#include "Artist.h"
#include <drogon/utils/Utilities.h>
#include <string>

using namespace drogon;
using namespace drogon_model::sqlite3;

const std::string Artist::Cols::_manga_id = "manga_id";
const std::string Artist::Cols::_person_id = "person_id";
const std::string Artist::primaryKeyName = "";
const bool Artist::hasPrimaryKey = false;
const std::string Artist::tableName = "artist";

const std::vector<typename Artist::MetaData> Artist::metaData_={
{"manga_id","uint64_t","integer",8,0,0,1},
{"person_id","uint64_t","integer",8,0,0,1}
};
const std::string &Artist::getColumnName(size_t index) noexcept(false)
{
    assert(index < metaData_.size());
    return metaData_[index].colName_;
}
Artist::Artist(const Row &r, const ssize_t indexOffset) noexcept
{
    if(indexOffset < 0)
    {
        if(!r["manga_id"].isNull())
        {
            mangaId_=std::make_shared<uint64_t>(r["manga_id"].as<uint64_t>());
        }
        if(!r["person_id"].isNull())
        {
            personId_=std::make_shared<uint64_t>(r["person_id"].as<uint64_t>());
        }
    }
    else
    {
        size_t offset = (size_t)indexOffset;
        if(offset + 2 > r.size())
        {
            LOG_FATAL << "Invalid SQL result for this model";
            return;
        }
        size_t index;
        index = offset + 0;
        if(!r[index].isNull())
        {
            mangaId_=std::make_shared<uint64_t>(r[index].as<uint64_t>());
        }
        index = offset + 1;
        if(!r[index].isNull())
        {
            personId_=std::make_shared<uint64_t>(r[index].as<uint64_t>());
        }
    }

}

Artist::Artist(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 2)
    {
        LOG_ERROR << "Bad masquerading vector";
        return;
    }
    if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
    {
        dirtyFlag_[0] = true;
        if(!pJson[pMasqueradingVector[0]].isNull())
        {
            mangaId_=std::make_shared<uint64_t>((uint64_t)pJson[pMasqueradingVector[0]].asUInt64());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        dirtyFlag_[1] = true;
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            personId_=std::make_shared<uint64_t>((uint64_t)pJson[pMasqueradingVector[1]].asUInt64());
        }
    }
}

Artist::Artist(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("manga_id"))
    {
        dirtyFlag_[0]=true;
        if(!pJson["manga_id"].isNull())
        {
            mangaId_=std::make_shared<uint64_t>((uint64_t)pJson["manga_id"].asUInt64());
        }
    }
    if(pJson.isMember("person_id"))
    {
        dirtyFlag_[1]=true;
        if(!pJson["person_id"].isNull())
        {
            personId_=std::make_shared<uint64_t>((uint64_t)pJson["person_id"].asUInt64());
        }
    }
}

void Artist::updateByMasqueradedJson(const Json::Value &pJson,
                                            const std::vector<std::string> &pMasqueradingVector) noexcept(false)
{
    if(pMasqueradingVector.size() != 2)
    {
        LOG_ERROR << "Bad masquerading vector";
        return;
    }
    if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
    {
        dirtyFlag_[0] = true;
        if(!pJson[pMasqueradingVector[0]].isNull())
        {
            mangaId_=std::make_shared<uint64_t>((uint64_t)pJson[pMasqueradingVector[0]].asUInt64());
        }
    }
    if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
    {
        dirtyFlag_[1] = true;
        if(!pJson[pMasqueradingVector[1]].isNull())
        {
            personId_=std::make_shared<uint64_t>((uint64_t)pJson[pMasqueradingVector[1]].asUInt64());
        }
    }
}
                                                                    
void Artist::updateByJson(const Json::Value &pJson) noexcept(false)
{
    if(pJson.isMember("manga_id"))
    {
        dirtyFlag_[0] = true;
        if(!pJson["manga_id"].isNull())
        {
            mangaId_=std::make_shared<uint64_t>((uint64_t)pJson["manga_id"].asUInt64());
        }
    }
    if(pJson.isMember("person_id"))
    {
        dirtyFlag_[1] = true;
        if(!pJson["person_id"].isNull())
        {
            personId_=std::make_shared<uint64_t>((uint64_t)pJson["person_id"].asUInt64());
        }
    }
}

const uint64_t &Artist::getValueOfMangaId() const noexcept
{
    const static uint64_t defaultValue = uint64_t();
    if(mangaId_)
        return *mangaId_;
    return defaultValue;
}
const std::shared_ptr<uint64_t> &Artist::getMangaId() const noexcept
{
    return mangaId_;
}
void Artist::setMangaId(const uint64_t &pMangaId) noexcept
{
    mangaId_ = std::make_shared<uint64_t>(pMangaId);
    dirtyFlag_[0] = true;
}




const uint64_t &Artist::getValueOfPersonId() const noexcept
{
    const static uint64_t defaultValue = uint64_t();
    if(personId_)
        return *personId_;
    return defaultValue;
}
const std::shared_ptr<uint64_t> &Artist::getPersonId() const noexcept
{
    return personId_;
}
void Artist::setPersonId(const uint64_t &pPersonId) noexcept
{
    personId_ = std::make_shared<uint64_t>(pPersonId);
    dirtyFlag_[1] = true;
}




void Artist::updateId(const uint64_t id)
{
}

const std::vector<std::string> &Artist::insertColumns() noexcept
{
    static const std::vector<std::string> inCols={
        "manga_id",
        "person_id"
    };
    return inCols;
}

void Artist::outputArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[0])
    {
        if(getMangaId())
        {
            binder << getValueOfMangaId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[1])
    {
        if(getPersonId())
        {
            binder << getValueOfPersonId();
        }
        else
        {
            binder << nullptr;
        }
    }
}

const std::vector<std::string> Artist::updateColumns() const
{
    std::vector<std::string> ret;
    if(dirtyFlag_[0])
    {
        ret.push_back(getColumnName(0));
    }
    if(dirtyFlag_[1])
    {
        ret.push_back(getColumnName(1));
    }
    return ret;
}

void Artist::updateArgs(drogon::orm::internal::SqlBinder &binder) const
{
    if(dirtyFlag_[0])
    {
        if(getMangaId())
        {
            binder << getValueOfMangaId();
        }
        else
        {
            binder << nullptr;
        }
    }
    if(dirtyFlag_[1])
    {
        if(getPersonId())
        {
            binder << getValueOfPersonId();
        }
        else
        {
            binder << nullptr;
        }
    }
}
Json::Value Artist::toJson() const
{
    Json::Value ret;
    if(getMangaId())
    {
        ret["manga_id"]=(Json::UInt64)getValueOfMangaId();
    }
    else
    {
        ret["manga_id"]=Json::Value();
    }
    if(getPersonId())
    {
        ret["person_id"]=(Json::UInt64)getValueOfPersonId();
    }
    else
    {
        ret["person_id"]=Json::Value();
    }
    return ret;
}

Json::Value Artist::toMasqueradedJson(
    const std::vector<std::string> &pMasqueradingVector) const
{
    Json::Value ret;
    if(pMasqueradingVector.size() == 2)
    {
        if(!pMasqueradingVector[0].empty())
        {
            if(getMangaId())
            {
                ret[pMasqueradingVector[0]]=(Json::UInt64)getValueOfMangaId();
            }
            else
            {
                ret[pMasqueradingVector[0]]=Json::Value();
            }
        }
        if(!pMasqueradingVector[1].empty())
        {
            if(getPersonId())
            {
                ret[pMasqueradingVector[1]]=(Json::UInt64)getValueOfPersonId();
            }
            else
            {
                ret[pMasqueradingVector[1]]=Json::Value();
            }
        }
        return ret;
    }
    LOG_ERROR << "Masquerade failed";
    if(getMangaId())
    {
        ret["manga_id"]=(Json::UInt64)getValueOfMangaId();
    }
    else
    {
        ret["manga_id"]=Json::Value();
    }
    if(getPersonId())
    {
        ret["person_id"]=(Json::UInt64)getValueOfPersonId();
    }
    else
    {
        ret["person_id"]=Json::Value();
    }
    return ret;
}

bool Artist::validateJsonForCreation(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("manga_id"))
    {
        if(!validJsonOfField(0, "manga_id", pJson["manga_id"], err, true))
            return false;
    }
    else
    {
        err="The manga_id column cannot be null";
        return false;
    }
    if(pJson.isMember("person_id"))
    {
        if(!validJsonOfField(1, "person_id", pJson["person_id"], err, true))
            return false;
    }
    else
    {
        err="The person_id column cannot be null";
        return false;
    }
    return true;
}
bool Artist::validateMasqueradedJsonForCreation(const Json::Value &pJson,
                                                const std::vector<std::string> &pMasqueradingVector,
                                                std::string &err)
{
    if(pMasqueradingVector.size() != 2)
    {
        err = "Bad masquerading vector";
        return false;
    }
    try {
      if(!pMasqueradingVector[0].empty())
      {
          if(pJson.isMember(pMasqueradingVector[0]))
          {
              if(!validJsonOfField(0, pMasqueradingVector[0], pJson[pMasqueradingVector[0]], err, true))
                  return false;
          }
        else
        {
            err="The " + pMasqueradingVector[0] + " column cannot be null";
            return false;
        }
      }
      if(!pMasqueradingVector[1].empty())
      {
          if(pJson.isMember(pMasqueradingVector[1]))
          {
              if(!validJsonOfField(1, pMasqueradingVector[1], pJson[pMasqueradingVector[1]], err, true))
                  return false;
          }
        else
        {
            err="The " + pMasqueradingVector[1] + " column cannot be null";
            return false;
        }
      }
    }
    catch(const Json::LogicError &e) 
    {
      err = e.what();
      return false;
    }
    return true;
}
bool Artist::validateJsonForUpdate(const Json::Value &pJson, std::string &err)
{
    if(pJson.isMember("manga_id"))
    {
        if(!validJsonOfField(0, "manga_id", pJson["manga_id"], err, false))
            return false;
    }
    if(pJson.isMember("person_id"))
    {
        if(!validJsonOfField(1, "person_id", pJson["person_id"], err, false))
            return false;
    }
    return true;
}
bool Artist::validateMasqueradedJsonForUpdate(const Json::Value &pJson,
                                              const std::vector<std::string> &pMasqueradingVector,
                                              std::string &err)
{
    if(pMasqueradingVector.size() != 2)
    {
        err = "Bad masquerading vector";
        return false;
    }
    try {
      if(!pMasqueradingVector[0].empty() && pJson.isMember(pMasqueradingVector[0]))
      {
          if(!validJsonOfField(0, pMasqueradingVector[0], pJson[pMasqueradingVector[0]], err, false))
              return false;
      }
      if(!pMasqueradingVector[1].empty() && pJson.isMember(pMasqueradingVector[1]))
      {
          if(!validJsonOfField(1, pMasqueradingVector[1], pJson[pMasqueradingVector[1]], err, false))
              return false;
      }
    }
    catch(const Json::LogicError &e) 
    {
      err = e.what();
      return false;
    }
    return true;
}
bool Artist::validJsonOfField(size_t index,
                              const std::string &fieldName,
                              const Json::Value &pJson, 
                              std::string &err, 
                              bool isForCreation)
{
    switch(index)
    {
        case 0:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(!pJson.isUInt64())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
        case 1:
            if(pJson.isNull())
            {
                err="The " + fieldName + " column cannot be null";
                return false;
            }
            if(!pJson.isUInt64())
            {
                err="Type error in the "+fieldName+" field";
                return false;
            }
            break;
     
        default:
            err="Internal error in the server";
            return false;
            break;
    }
    return true;
}
