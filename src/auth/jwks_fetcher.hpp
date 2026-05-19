//
// Created by Saadat Baig on 19.05.26.
//
#pragma once

#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <jwt-cpp/jwt.h>
#include <string>



namespace Metallic::Authorization
{
    using JwksKeySet = jwt::jwks<jwt::traits::nlohmann_json>;

    // Fetches the JWKS from <issuer>/protocol/openid-connect/certs.
    // issuer must include scheme, e.g. "https://keycloak.example.com/realms/myrealm"
    //                                or "http://localhost:8080/realms/myrealm"
    // Throws std::runtime_error on failure.
    JwksKeySet fetch_jwks(std::string const& issuer);
}
