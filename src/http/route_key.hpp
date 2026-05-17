//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "types.hpp"

#include <string>


namespace Metallic
{
    struct RouteKey
    {
        http::verb method;
        std::string path;

        bool operator==(RouteKey const&) const = default;
    };

    struct RouteKeyHash
    {
        std::size_t operator()(RouteKey const& key) const noexcept
        {
            auto h1 = std::hash<int>{}(static_cast<int>(key.method));
            auto h2 = std::hash<std::string>{}(key.path);

            return h1 ^ (h2 << 1);
        }
    };

}
