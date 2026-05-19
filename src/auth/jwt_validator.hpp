//
// Created by Saadat Baig on 19.05.26.
//
#pragma once

#include "jwks_fetcher.hpp"
#include "jwt_claims.hpp"

#include <optional>
#include <string>
#include <string_view>


namespace Metallic::Authorization
{
    // Validates a raw Bearer token string against the provided JWKS.
    // Returns JwtClaims on success, std::nullopt if the token is missing, malformed, or invalid.
    std::optional<JwtClaims> validate_token(std::string_view bearer_token, JwksKeySet const& jwks);
}