//
// Created by Saadat Baig on 16.05.26.
//
#pragma once

#include "environment_defs.hpp"
#include "environment_loader.hpp"


static const EnvironmentConfiguration g_config_security({
    {APP_SECURITY_LOGIN_ENABLED, "APP_ENABLE_LOGIN", "false"},
    {APP_SECURITY_OAUTH2_ISSUER_URL, "APP_KC_ISSUER", ""},
});

static const EnvironmentConfiguration g_config_server({
    {APP_SERVER_PORT, "APP_PORT", "8090"},
    {APP_SERVER_BODY_LIMIT, "APP_MAX_REQUEST_BYTES", "10485760"}, // 10 MB default
});
