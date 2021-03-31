#pragma once

#include "Types.h"
#include <vector>
#include <drogon/drogon.h>

bool makeThumbnail(CSR name, const drogon::HttpFile& httpFile);

std::vector<std::string> splitCSV(const std::string& str);

void splitCSV (
    const std::string& str,
    std::vector<std::string>& in,
    std::vector<std::string>& ex
);

std::string joinVec(const std::vector<std::string>& vec);
