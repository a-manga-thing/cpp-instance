#pragma once

#include "Types.h"
#include <tuple>
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

std::string decrypt (CSR str, CSR key, CSR pass);
std::string encrypt (CSR id, CSR str, CSR pub);
std::tuple<std::string, std::string, std::string> makeKeyPair(CSR str);
