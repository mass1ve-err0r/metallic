//
// Created by Saadat Baig on 19.05.26.
//
#include "test_secured_controller.hpp"

#include "../http/response.hpp"


namespace Metallic::Controllers
{
    asio::awaitable<Response>
    TestSecuredController::get(RequestContext& ctx)
    {
        co_return json_response(
            ctx.request,
            http::status::ok,
            R"({"status":"ok"})"
        );
    }

}
