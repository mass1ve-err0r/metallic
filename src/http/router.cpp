//
// Created by Saadat Baig on 17.05.26.
//
#include "router.hpp"
#include "response.hpp"


namespace Metallic
{
    void
    Router::get(std::string path, Handler handler)
    {
        add(http::verb::get, std::move(path), std::move(handler));
    }

    void
    Router::post(std::string path, Handler handler)
    {
        add(http::verb::post, std::move(path), std::move(handler));
    }

    void
    Router::put(std::string path, Handler handler)
    {
        add(http::verb::put, std::move(path), std::move(handler));
    }

    void
    Router::del(std::string path, Handler handler)
    {
        add(http::verb::delete_, std::move(path), std::move(handler));
    }

    void
    Router::add(http::verb method, std::string path, Handler handler)
    {
        routes_.emplace(
            RouteKey{
                .method = method,
                .path = std::move(path)
            },
            std::move(handler)
        );
    }

    asio::awaitable<Response>
    Router::dispatch(RequestContext& ctx) const
    {
        auto const target = ctx.request.target();

        RouteKey key{
            .method = ctx.request.method(),
            .path = std::string{target}
        };

        auto it = routes_.find(key);

        if (it == routes_.end()) {
            co_return not_found(ctx.request);
        }

        co_return co_await it->second(ctx);
    }

}
