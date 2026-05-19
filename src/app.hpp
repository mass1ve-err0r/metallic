//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "http/router.hpp"
#include "auth/jwks_fetcher.hpp"


namespace Metallic
{
    class App
    {
    public:
        Router& router();
        Router const& router() const;

        void set_jwks(Authorization::JwksKeySet jwks);
        Authorization::JwksKeySet const& jwks() const;

        asio::awaitable<Response> handle(RequestContext& ctx) const;

    private:
        Router router_;
        Authorization::JwksKeySet jwks_;
    };

}
