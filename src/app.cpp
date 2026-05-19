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
    App::set_jwks(Auth::JwksKeySet jwks)
    {
        jwks_ = std::move(jwks);
    }

    Auth::JwksKeySet const&
    App::jwks() const
    {
        return jwks_;
    }

    asio::awaitable<Response>
    App::handle(RequestContext& ctx) const
    {
        co_return co_await router_.dispatch(ctx);
    }

}
