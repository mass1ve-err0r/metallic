//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "body_view.hpp"
#include "types.hpp"
#include "../auth/jwt_claims.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>


namespace Metallic
{
    class App; // forward declaration — App owns the session that owns this context

    struct RequestContext
    {
        Request& request;
        tcp::endpoint remote_endpoint;
        std::optional<Authorization::JwtClaims> claims;
        // Raw pointer is safe: App is held alive by shared_ptr in session()
        // for the entire coroutine lifetime that this RequestContext exists in.
        App const* app{nullptr};

        [[nodiscard]] BodyView
        body() const noexcept
        {
            return BodyView{request.body()};
        }

        [[nodiscard]] std::span<std::uint8_t const>
        body_bytes() const noexcept
        {
            return body().bytes();
        }

        [[nodiscard]] std::string_view
        body_string_view() const noexcept
        {
            return body().as_string_view();
        }

        [[nodiscard]] std::string
        body_string() const
        {
            return body().to_string();
        }
    };

}
