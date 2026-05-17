//
// Created by Saadat Baig on 17.05.26.
//
#include "listener.hpp"
#include "session.hpp"

#include <spdlog/spdlog.h>


namespace Metallic::Server
{

    asio::awaitable<void>
    listener(tcp::endpoint endpoint, std::shared_ptr<App const> app)
    {
        auto executor = co_await asio::this_coro::executor;

        tcp::acceptor acceptor{executor};
        acceptor.open(endpoint.protocol());
        acceptor.set_option(asio::socket_base::reuse_address(true));
        acceptor.bind(endpoint);
        acceptor.listen(asio::socket_base::max_listen_connections);

        spdlog::info(
            "listening on {}:{}",
            endpoint.address().to_string(),
            endpoint.port()
        );

        for (;;) {
            auto [ec, socket] = co_await acceptor.async_accept(token);

            if (ec) {
                spdlog::warn("accept failed: {}", ec.message());
                continue;
            }

            asio::co_spawn(
                executor,
                session(std::move(socket), app),
                asio::detached
            );
        }
    }

}
