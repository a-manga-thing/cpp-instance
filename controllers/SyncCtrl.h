#pragma once

#include "Types.h"
#include "Manga.h"
#include "Chapter.h"
#include "BaseCtrl.h"
#include <drogon/HttpController.h>
#include <drogon/orm/RestfulController.h>

using namespace drogon;

using namespace drogon_model::sqlite3;

class SyncCtrl: public HttpController<SyncCtrl>, public BaseCtrl
{
public:
    
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(SyncCtrl::subscribe,"/sync/subscribe?address={1}",Get,Options);
    ADD_METHOD_TO(SyncCtrl::accept,"/sync/accept?address={1}",Get,Options);
    ADD_METHOD_TO(SyncCtrl::push,"/sync/push",Post,Options);
    METHOD_LIST_END
    
    void subscribe(const HttpRequestPtr& req, HttpCallback&& callback, CSR id);
    void accept(const HttpRequestPtr& req, HttpCallback&& callback, CSR id);
    void push(const HttpRequestPtr& req, HttpCallback&& callback);
};
