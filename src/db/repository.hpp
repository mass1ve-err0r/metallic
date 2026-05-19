//
// Created by Saadat Baig on 19.05.26.
//
#pragma once

#include "error.hpp"
#include "entity.hpp"
#include "pool.hpp"

#include <pqxx/pqxx>

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>


namespace Metallic::Persistence
{
    template<typename T>
    requires std::derived_from<T, Entity<T>>
    class Repository
    {
    public:
        explicit Repository(std::shared_ptr<DatabasePool> pool)
            : pool_{std::move(pool)} {}

        asio::awaitable<std::optional<T>> find_by_id(std::int64_t id)
        {
            co_return co_await pool_->execute([&](ConnectionGuard guard) -> std::optional<T> {
                try {
                    pqxx::work tx{guard.get()};
                    auto result = tx.exec_params(
                        "SELECT * FROM " + std::string{T::table_name} + " WHERE id = $1 LIMIT 1",
                        id
                    );
                    tx.commit();

                    if (result.empty()) return std::nullopt;
                    return T::from_row(result[0]);

                } catch (pqxx::sql_error const& e) {
                    throw DbError{e.what()};
                }
            });
        }

        asio::awaitable<std::vector<T>> find_all()
        {
            co_return co_await pool_->execute([&](ConnectionGuard guard) -> std::vector<T> {
                try {
                    pqxx::work tx{guard.get()};
                    auto result = tx.exec("SELECT * FROM " + std::string{T::table_name});
                    tx.commit();

                    std::vector<T> out;
                    out.reserve(result.size());
                    for (auto const& row : result) {
                        out.push_back(T::from_row(row));
                    }
                    return out;

                } catch (pqxx::sql_error const& e) {
                    throw DbError{e.what()};
                }
            });
        }

        asio::awaitable<T> insert(T entity)
        {
            co_return co_await pool_->execute([entity = std::move(entity)](ConnectionGuard guard) -> T {
                try {
                    constexpr auto names = T::field_names();
                    constexpr auto N = names.size();

                    auto params = entity.to_params();

                    // Build: INSERT INTO table (col1, col2, ...) VALUES ($1, $2, ...) RETURNING *
                    std::string cols, placeholders;
                    for (std::size_t i = 0; i < N; ++i) {
                        if (i > 0) { cols += ", "; placeholders += ", "; }
                        cols += names[i];
                        placeholders += "$" + std::to_string(i + 1);
                    }

                    std::string sql = "INSERT INTO " + std::string{T::table_name}
                        + " (" + cols + ") VALUES (" + placeholders + ") RETURNING *";

                    pqxx::work tx{guard.get()};
                    pqxx::params p;
                    for (auto const& v : params) p.append(v);
                    auto result = tx.exec_params(sql, p);
                    tx.commit();

                    if (result.empty()) throw DbError{"insert returned no rows"};
                    return T::from_row(result[0]);

                } catch (pqxx::sql_error const& e) {
                    throw DbError{e.what()};
                }
            });
        }

        asio::awaitable<std::optional<T>> update(std::int64_t id, T entity)
        {
            co_return co_await pool_->execute([id, entity = std::move(entity)](ConnectionGuard guard) -> std::optional<T> {
                try {
                    constexpr auto names = T::field_names();
                    constexpr auto N = names.size();

                    auto params = entity.to_params();

                    // Build: UPDATE table SET col1=$1, col2=$2, ... WHERE id=$N+1 RETURNING *
                    std::string sets;
                    for (std::size_t i = 0; i < N; ++i) {
                        if (i > 0) sets += ", ";
                        sets += std::string{names[i]} + " = $" + std::to_string(i + 1);
                    }

                    std::string sql = "UPDATE " + std::string{T::table_name}
                        + " SET " + sets
                        + " WHERE id = $" + std::to_string(N + 1)
                        + " RETURNING *";

                    pqxx::work tx{guard.get()};
                    pqxx::params p;
                    for (auto const& v : params) p.append(v);
                    p.append(id);
                    auto result = tx.exec_params(sql, p);
                    tx.commit();

                    if (result.empty()) return std::nullopt;
                    return T::from_row(result[0]);

                } catch (pqxx::sql_error const& e) {
                    throw DbError{e.what()};
                }
            });
        }

        asio::awaitable<bool> remove(std::int64_t id)
        {
            co_return co_await pool_->execute([id](ConnectionGuard guard) -> bool {
                try {
                    pqxx::work tx{guard.get()};
                    auto result = tx.exec_params(
                        "DELETE FROM " + std::string{T::table_name} + " WHERE id = $1",
                        id
                    );
                    tx.commit();
                    return result.affected_rows() > 0;

                } catch (pqxx::sql_error const& e) {
                    throw DbError{e.what()};
                }
            });
        }

    private:
        std::shared_ptr<DatabasePool> pool_;
    };

}
