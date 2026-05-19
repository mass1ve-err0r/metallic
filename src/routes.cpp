//
// Created by Saadat Baig on 17.05.26.
//
#include "routes.hpp"

#include "auth/require_authorization.hpp"
#include "controllers/health_controller.hpp"
#include "controllers/test_secured_controller.hpp"


namespace Metallic
{
    void
    register_routes(App& app)
    {
        app.router().get("/api/health", Controllers::HealthController::get);
        app.router().get("/api/secured/test",Authorization::require_authorization(app.jwks(), Controllers::TestSecuredController::get));
    }

}
