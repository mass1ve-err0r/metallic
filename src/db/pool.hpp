//
// Created by Saadat Baig on 19.05.26.
//
//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "error.hpp"

#include <pqxx/pqxx>
#include <boost/asio.hpp>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>


namespace Metallic::Persistence
{
    namespace asio = boost::asio;

    class DatabasePool;

    // RAII connection guard — returns connection to pool on destruction
    class ConnectionGuard
    {
    public:
        ConnectionGuard(std::shared_ptr<DatabasePool> pool, std::unique_ptr<pqxx::connection> conn)
            : pool_{std::move(pool)}, conn_{std::move(conn)} {}

        ~ConnectionGuard();

        ConnectionGuard(ConnectionGuard&&) = default;
        ConnectionGuard& operator=(ConnectionGuard&&) = default;

        ConnectionGuard(ConnectionGuard const&) = delete;
        ConnectionGuard& operator=(ConnectionGuard const&) = delete;

        pqxx::connection& get() noexcept { return *conn_; }
        pqxx::connection* operator->() noexcept { return conn_.get(); }

    private:
        std::shared_ptr<DatabasePool> pool_;
        std::unique_ptr<pqxx::connection> conn_;
    };

    class DatabasePool : public std::enable_shared_from_this<DatabasePool>
    {
    public:
        DatabasePool(std::string const& connection_string, std::size_t size);

        // Blocks until a connection is available, then returns it as a RAII guard.
        // Must be called from the db thread pool executor via asio::post.
        ConnectionGuard acquire();

        // Post a db task as an awaitable — runs on the internal thread pool
        template<typename F>
        asio::awaitable<std::invoke_result_t<F, ConnectionGuard>> execute(F&& f)
        {
            auto self = shared_from_this();
            co_return co_await asio::post(
                thread_pool_,
                asio::use_awaitable,
                [self, f = std::forward<F>(f)]() mutable {
                    return f(self->acquire());
                }
            );
        }

        void release(std::unique_ptr<pqxx::connection> conn);

        asio::thread_pool& executor() { return thread_pool_; }

    private:
        std::queue<std::unique_ptr<pqxx::connection>> connections_;
        std::mutex mutex_;
        std::condition_variable cv_;
        asio::thread_pool thread_pool_;
    };

}
