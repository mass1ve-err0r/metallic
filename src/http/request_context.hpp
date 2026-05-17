//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "body_view.hpp"
#include "types.hpp"

#include <span>
#include <string>
#include <string_view>


namespace Metallic
{
    struct RequestContext
    {
        Request& request;
        tcp::endpoint remote_endpoint;

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
