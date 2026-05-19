//
// Created by Saadat Baig on 17.05.26.
//
#include "app.hpp"
#include "auth/jwks_fetcher.hpp"
#include "db/pool.hpp"
#include "http/types.hpp"
#include "logging/logger.hpp"
#include "routes.hpp"
#include "server/listener.hpp"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "environment.hpp"


static std::optional<Metallic::Authorization::JwksKeySet>
setup_web_security()
{
    auto isLoginEnabled = g_config_security.get_optional(APP_SECURITY_LOGIN_ENABLED).value_or("false") == "true";
    if (!isLoginEnabled)
    {
        spdlog::info("Setting up server with PERMISSIVE security handling, use for local development only!");
        return std::nullopt;
    }

    auto kc_issuer = g_config_security.get_optional(APP_SECURITY_OAUTH2_ISSUER_URL);
    if (!kc_issuer) {
        throw std::runtime_error("APP_KC_ISSUER environment variable not set!");
    }

    spdlog::info("fetching JWKS from {}", kc_issuer.value());
    auto jwks = Metallic::Authorization::fetch_jwks(kc_issuer.value());
    spdlog::info("JWKS loaded successfully");

    return jwks;
}

static std::optional<std::shared_ptr<Metallic::Persistence::DatabasePool>>
setup_db()
{
    auto db_url = g_config_db.get_optional(APP_DB_URL);
    if (!db_url) {
        spdlog::info("DB URL environment variable not set, not using any persistnce!");
        return std::nullopt;
    }

    auto pool_size = g_config_db.get_as<std::size_t>(APP_DB_POOL_SIZE);

    spdlog::info("connecting to database (pool size: {})", pool_size);
    auto pool = std::make_shared<Metallic::Persistence::DatabasePool>(db_url.value(), pool_size);
    spdlog::info("database pool ready");

    return pool;
}

int
main(int argc, char* argv[])
{
    try
    {
        Metallic::Logging::Logger::init();

        auto server = std::make_shared<Metallic::App>();

        auto jwks = setup_web_security();
        if (jwks)
        {
            server->set_jwks(std::move(jwks.value()));
        }

        auto db = setup_db();
        if (db)
        {
            server->set_db_pool(db.value());
        }

        Metallic::register_routes(*server);

        auto const address = Metallic::asio::ip::make_address(argc > 1 ? argv[1] : "0.0.0.0");
        auto const port = static_cast<unsigned short>(argc > 2 ? std::atoi(argv[2]) : g_config_server.get_as<uint16_t>(APP_SERVER_PORT));
        auto const thread_count =
            std::max(1u, argc > 3
                ? static_cast<unsigned>(std::atoi(argv[3]))
                : std::thread::hardware_concurrency());

        Metallic::asio::io_context ioc{static_cast<int>(thread_count)};

        Metallic::asio::co_spawn(
            ioc,
            Metallic::Server::listener(
                Metallic::tcp::endpoint{address, port},
                server
            ),
            Metallic::asio::detached
        );

        spdlog::info("starting server with {} io thread(s)", thread_count);

        std::vector<std::thread> threads;
        threads.reserve(thread_count - 1);

        for (unsigned i = 1; i < thread_count; ++i)
        {
            threads.emplace_back([&ioc] {
                ioc.run();
            });
        }

        ioc.run();

        for (auto& thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        Metallic::Logging::Logger::shutdown();
    } catch (std::exception const& e)
    {
        std::cerr << "fatal: " << e.what() << '\n';
        Metallic::Logging::Logger::shutdown();
        return 1;
    }

}
