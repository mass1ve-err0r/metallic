//
// Created by Saadat Baig on 19.05.26.
//
#pragma once

#include <nlohmann/json.hpp>
#include <map>
#include <string>


namespace Metallic::Authorization
{
    struct JwtClaims
    {
        std::string sub;
        std::map<std::string, nlohmann::json> all;
    };
}