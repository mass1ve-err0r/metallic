//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "../http/handler.hpp"


namespace Metallic::Controllers
{
    class HealthController
    {
    public:
        static asio::awaitable<Response> get(RequestContext& ctx);
    };

}
