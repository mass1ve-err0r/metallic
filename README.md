# metallic
> Async HTTP/1.1 server built on Boost.Beast + Asio coroutines | C++23

- Multi-threaded `io_context` with coroutine-per-session
- JWT/OIDC resource server via Keycloak JWKS (RS256)
- PostgreSQL connection pool backed by `asio::thread_pool`
- Boost.PFR aggregate reflection for zero-boilerplate ORM

---

## Environment variables

### Server

| Variable | Default | Description |
|---|---|---|
| `APP_PORT` | `8090` | Listen port |
| `APP_MAX_REQUEST_BYTES` | `10485760` | Max request body (10 MB) |

### Security

| Variable | Default | Description |
|---|---|---|
| `APP_ENABLE_LOGIN` | `false` | Set `true` to enforce JWT validation. When `false`, all routes are permissive. |
| `APP_KC_ISSUER` | — | Full Keycloak issuer URL, e.g. `https://keycloak.example.com/realms/myrealm`. Required when login is enabled. |

JWKS is fetched once at startup from `$APP_KC_ISSUER/protocol/openid-connect/certs`. Both `http://` and `https://` issuers are supported — use `http://` for local Keycloak.

### Database

| Variable | Default | Description |
|---|---|---|
| `APP_DB_URL` | — | libpq connection string, e.g. `postgresql://user:pass@localhost/mydb`. Optional — omit to run without a database. |
| `APP_DB_POOL_SIZE` | `8` | Number of PostgreSQL connections. Thread pool runs at `2x` this value to avoid deadlock under full contention. |

---

## Routing

Routes are registered in `src/routes.cpp`:

```cpp
void register_routes(App& app)
{
    app.router().get("/health",  Controllers::HealthController::get);
    app.router().post("/users",  Controllers::UserController::create);
}
```

Supported verbs: `get`, `post`, `put`, `del`.

---

## Securing routes

Wrap any handler with `require_authorization`. Pass `app.jwks()` at registration time — the key set is captured by value, so the handler is self-contained.

```cpp
#include "auth/require_auth.hpp"

void register_routes(App& app)
{
    app.router().get("/me", Authorization::require_authorization(app.jwks(), Controllers::UserController::me));
}
```

On failure, the wrapper returns before the inner handler is called:

- Missing or malformed `Authorization` header → `401 {"error":"missing_token"}`
- Invalid or expired token → `401 {"error":"invalid_token"}`

Inside a secured handler, the full decoded JWT payload is available via `ctx.claims`:

```cpp
asio::awaitable<Response> me(RequestContext& ctx)
{
    auto& claims = ctx.claims.value(); // safe — require_authorization guarantees this is set

    auto sub   = claims.sub;
    auto email = claims.all.at("email").get<std::string>();
    auto roles = claims.all.at("realm_access")["roles"]; // nlohmann::json array

    co_return json_response(ctx.request, http::status::ok,
        R"({"sub":")" + sub + R"("})");
}
```

`claims.all` is a `std::map<std::string, nlohmann::json>` — the full token payload. Any claim is accessible, including nested objects.

---

## Defining entities

An entity is a plain aggregate struct. `METALLIC_ENTITY` registers field names for SQL mapping. The struct name does **not** drive the table name — `table_name` does, so you control it explicitly.

```cpp
#include "db/entity.hpp"

struct User : Metallic::Db::Entity<User>
{
    static constexpr auto table_name = "users";

    int64_t     id;
    std::string name;
    std::string email;

    METALLIC_ENTITY(id, name, email)
};
```

Rules:
- Must be an aggregate (no user-declared constructors, no private members, no base classes other than `Entity<T>`)
- Field order in `METALLIC_ENTITY` must match declaration order — a `static_assert` fires at compile time if counts diverge
- Up to 16 fields supported; extend the macro chain in `entity.hpp` if needed
- `std::optional<T>` fields are supported via `field_cast_opt` for nullable columns

---

## Repository

`Repository<T>` provides the five standard operations. Construct it per-request from `ctx.app->db_pool()` — the cost is one atomic refcount increment.

```cpp
#include "db/repository.hpp"

Metallic::Db::Repository<User> repo{ctx.app->db_pool()};
```

All methods are `co_await`-able and run on the db thread pool, never blocking the io thread.

| Method | Signature | Notes |
|---|---|---|
| `find_by_id` | `co_await repo.find_by_id(id)` → `std::optional<T>` | `SELECT * WHERE id = $1 LIMIT 1` |
| `find_all` | `co_await repo.find_all()` → `std::vector<T>` | `SELECT *` — no pagination |
| `insert` | `co_await repo.insert(entity)` → `T` | `INSERT ... RETURNING *` — returns the persisted row |
| `update` | `co_await repo.update(id, entity)` → `std::optional<T>` | `UPDATE ... WHERE id = $1 RETURNING *` — `nullopt` if not found |
| `remove` | `co_await repo.remove(id)` → `bool` | `true` if a row was deleted |

---

## Controller examples

### Public endpoint

```cpp
// src/controllers/health_controller.hpp
#pragma once
#include "../http/request_context.hpp"
#include "../http/response.hpp"

namespace Metallic::Controllers
{
    struct HealthController
    {
        static asio::awaitable<Response> get(RequestContext& ctx)
        {
            co_return json_response(ctx.request, http::status::ok, R"({"status":"ok"})");
        }
    };
}
```

### Secured endpoint, no database

```cpp
// src/controllers/profile_controller.hpp
#pragma once
#include "../http/request_context.hpp"
#include "../http/response.hpp"

namespace Metallic::Controllers
{
    struct ProfileController
    {
        static asio::awaitable<Response> me(RequestContext& ctx)
        {
            auto& claims = ctx.claims.value();

            std::string body = R"({"sub":")" + claims.sub + R"("})";
            co_return json_response(ctx.request, http::status::ok, body);
        }
    };
}
```

```cpp
// src/routes.cpp
app.router().get("/me", Authorization::require_authorization(app.jwks(), Controllers::ProfileController::me));
```

### Secured endpoint with database

```cpp
// src/controllers/user_controller.hpp
#pragma once
#include "../db/repository.hpp"
#include "../http/request_context.hpp"
#include "../http/response.hpp"
#include "../models/user.hpp"

namespace Metallic::Controllers
{
    struct UserController
    {
        static asio::awaitable<Response> get_by_id(RequestContext& ctx)
        {
            // parse id from query string or path — metallic has no path params yet,
            // pass via query string: GET /users?id=1
            auto target = std::string{ctx.request.target()};
            auto pos    = target.find("?id=");
            if (pos == std::string::npos) {
                co_return json_response(ctx.request, http::status::bad_request,
                    R"({"error":"missing_id"})");
            }

            int64_t id = std::stoll(target.substr(pos + 4));

            Metallic::Db::Repository<User> repo{ctx.app->db_pool()};
            auto user = co_await repo.find_by_id(id);

            if (!user) {
                co_return json_response(ctx.request, http::status::not_found,
                    R"({"error":"not_found"})");
            }

            std::string body = R"({"id":)" + std::to_string(user->id)
                + R"(,"name":")" + user->name + R"(","email":")" + user->email + R"("})";

            co_return json_response(ctx.request, http::status::ok, body);
        }

        static asio::awaitable<Response> create(RequestContext& ctx)
        {
            // parse body manually or with nlohmann::json
            auto json = nlohmann::json::parse(ctx.body_string(), nullptr, false);
            if (json.is_discarded()) {
                co_return json_response(ctx.request, http::status::bad_request,
                    R"({"error":"invalid_json"})");
            }

            User u{
                .id    = 0, // db assigns
                .name  = json.value("name", ""),
                .email = json.value("email", ""),
            };

            Metallic::Db::Repository<User> repo{ctx.app->db_pool()};
            auto created = co_await repo.insert(std::move(u));

            std::string body = R"({"id":)" + std::to_string(created.id) + R"(})";
            co_return json_response(ctx.request, http::status::created, body);
        }
    };
}
```

```cpp
// src/routes.cpp
app.router().get("/users",  Authorization::require_authorization(app.jwks(), Controllers::UserController::get_by_id));
app.router().post("/users", Authorization::require_authorization(app.jwks(), Controllers::UserController::create));
```

---

## Build

Requires: CMake 3.25+, a C++23 compiler, `libpq-dev`, `libcurl-dev`.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Dependencies fetched automatically via `FetchContent`: Boost, spdlog, jwt-cpp, nlohmann_json, Boost.PFR, libpqxx.