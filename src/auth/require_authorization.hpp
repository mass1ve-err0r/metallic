//
// Created by Saadat Baig on 19.05.26.
//
#pragma once

#include "environment.hpp"
#include "jwks_fetcher.hpp"
#include "jwt_validator.hpp"
#include "../http/handler.hpp"
#include "../http/response.hpp"

#include <spdlog/spdlog.h>


namespace Metallic::Auth
{

    inline Handler
    require_auth(JwksKeySet const& jwks, Handler inner)
    {
        auto isLoginEnabled = g_config_security.get_optional(APP_SECURITY_LOGIN_ENABLED).value_or("false") == "true";

        return [isLoginEnabled, jwks = std::move(jwks), inner = std::move(inner)](RequestContext& ctx) -> asio::awaitable<Response> {
            if (!isLoginEnabled)
            {
                co_return co_await inner(ctx);
            }

            auto const auth_header = ctx.request[http::field::authorization];

            if (auth_header.empty() || !auth_header.starts_with("Bearer "))
            {
                co_return json_response(ctx.request, http::status::unauthorized,
                    R"({"error":"missing_token"})");
            }

            auto token = auth_header.substr(7); // strip "Bearer "
            auto claims = validate_token(token, jwks);

            if (!claims)
            {
                co_return json_response(ctx.request, http::status::unauthorized,
                    R"({"error":"invalid_token"})");
            }

            ctx.claims = std::move(claims);

            co_return co_await inner(ctx);
        };

    }

}
