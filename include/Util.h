#pragma once

#include "Types.h"
#include <vector>
#include <utility>
#include <drogon/drogon.h>

bool makeThumbnail(CSR name, const drogon::HttpFile& httpFile);

std::vector<std::string> splitCSV(const std::string& str);

void splitCSV (
    const std::string& str,
    std::vector<std::string>& in,
    std::vector<std::string>& ex
);

std::string joinVec(const std::vector<std::string>& vec);

void badRequest (
    HttpCallback& callback,
    CSR err,
    drogon::HttpStatusCode code = drogon::k400BadRequest
);

std::string desencrypt (CSR str);
std::pair<std::string, std::string> makeKeyPair();
