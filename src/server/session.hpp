//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "../app.hpp"
#include "../http/types.hpp"

#include <memory>


namespace Metallic::Server
{
    asio::awaitable<void>
    session(tcp::socket socket, std::shared_ptr<App const> app);

}
