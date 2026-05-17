//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "http/router.hpp"


namespace Metallic
{
    class App
    {
    public:
        Router& router();
        Router const& router() const;

        asio::awaitable<Response> handle(RequestContext& ctx) const;

    private:
        Router router_;
    };

}
