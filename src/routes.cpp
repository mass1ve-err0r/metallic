//
// Created by Saadat Baig on 17.05.26.
//
#include "routes.hpp"

#include "controllers/health_controller.hpp"


namespace Metallic
{
    void
    register_routes(App& app)
    {
        app.router().get("/health", Controllers::HealthController::get);
    }

}
