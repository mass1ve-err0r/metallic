//
// Created by Saadat Baig on 19.05.26.
//
#pragma once

#include "../http/handler.hpp"


namespace Metallic::Controllers
{
    class TestSecuredController
    {
    public:
        static asio::awaitable<Response> get(RequestContext& ctx);
    };

}
