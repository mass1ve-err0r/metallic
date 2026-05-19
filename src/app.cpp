//
// Created by Saadat Baig on 17.05.26.
//
#include "app.hpp"


namespace Metallic
{
    Router&
    App::router()
    {
        return router_;
    }

    Router const&
    App::router() const
    {
        return router_;
    }

    void
    App::set_jwks(Authorization::JwksKeySet jwks)
    {
        jwks_ = std::move(jwks);
    }

    Authorization::JwksKeySet const&
    App::jwks() const
    {
        return jwks_;
    }

    void
    App::set_db_pool(std::shared_ptr<Persistence::DatabasePool> pool)
    {
        db_pool_ = std::move(pool);
    }

    std::shared_ptr<Persistence::DatabasePool>
    App::db_pool() const
    {
        return db_pool_;
    }

    asio::awaitable<Response>
    App::handle(RequestContext& ctx) const
    {
        ctx.app = this;
        co_return co_await router_.dispatch(ctx);
    }

}
