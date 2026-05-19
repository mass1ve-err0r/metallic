//
// Created by Saadat Baig on 19.05.26.
//
#include "pool.hpp"

#include <spdlog/spdlog.h>


namespace Metallic::Persistence
{

    DatabasePool::DatabasePool(std::string const& connection_string, std::size_t size)
        : thread_pool_{size * 2}
    {
        for (std::size_t i = 0; i < size; ++i) {
            connections_.push(std::make_unique<pqxx::connection>(connection_string));
        }
        spdlog::info("db pool initialized with {} connection(s)", size);
    }

    ConnectionGuard DatabasePool::acquire()
    {
        std::unique_lock lock{mutex_};
        cv_.wait(lock, [this] { return !connections_.empty(); });

        auto conn = std::move(connections_.front());
        connections_.pop();

        return ConnectionGuard{shared_from_this(), std::move(conn)};
    }

    void DatabasePool::release(std::unique_ptr<pqxx::connection> conn)
    {
        {
            std::lock_guard lock{mutex_};
            connections_.push(std::move(conn));
        }
        cv_.notify_one();
    }

    ConnectionGuard::~ConnectionGuard()
    {
        if (conn_ && pool_) {
            pool_->release(std::move(conn_));
        }
    }

}
