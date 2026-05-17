//
// Created by Saadat Baig on 17.05.26.
//
#include "health_controller.hpp"

#include "../http/response.hpp"


namespace Metallic::Controllers
{
    asio::awaitable<Response>
    HealthController::get(RequestContext& ctx)
    {
        co_return json_response(
            ctx.request,
            http::status::ok,
            R"({"status":"ok"})"
        );
    }

}
