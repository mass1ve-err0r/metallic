//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "handler.hpp"
#include "route_key.hpp"

#include <unordered_map>


namespace Metallic
{
    class Router
    {
    public:
        void get(std::string path, Handler handler);
        void post(std::string path, Handler handler);
        void put(std::string path, Handler handler);
        void del(std::string path, Handler handler);

        asio::awaitable<Response> dispatch(RequestContext& ctx) const;

    private:
        void add(http::verb method, std::string path, Handler handler);

        std::unordered_map<RouteKey, Handler, RouteKeyHash> routes_;
    };

}
