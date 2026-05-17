//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "request_context.hpp"

#include <functional>


namespace Metallic
{
    using Handler = std::function<asio::awaitable<Response>(RequestContext&)>;
}
