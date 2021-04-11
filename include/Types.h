#pragma once

#include <string>
#include <drogon/HttpController.h>

using JSP = std::shared_ptr<Json::Value>;
using CJR = const Json::Value&;  //const json ref
using CJPR = const std::shared_ptr<Json::Value>&;  //const json ptr ref
using HttpCallback = std::function<void(const drogon::HttpResponsePtr&)>;
using CSR = const std::string&;  //const string ref

struct Instance {
    std::string url;
    std::string pass;
    std::string publicKey;
    std::string privateKey;
};
