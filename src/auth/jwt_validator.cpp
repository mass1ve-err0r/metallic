//
// Created by Saadat Baig on 19.05.26.
//
#define JWT_DISABLE_PICOJSON


#include "jwt_validator.hpp"

#include <jwt-cpp/traits/nlohmann-json/traits.h>
#include <jwt-cpp/jwt.h>


namespace Metallic::Auth
{
    std::optional<JwtClaims>
    validate_token(std::string_view bearer_token, JwksKeySet const& jwks)
    {
        try
        {
            auto decoded = jwt::decode<jwt::traits::nlohmann_json>(std::string{bearer_token});

            auto const kid = decoded.get_key_id();
            if (!jwks.has_jwk(kid))
            {
                return std::nullopt;
            }

            auto const& jwk = jwks.get_jwk(kid);

            auto const public_key = jwt::helper::create_public_key_from_rsa_components(
                jwk.get_jwk_claim("n").as_string(),
                jwk.get_jwk_claim("e").as_string()
            );

            auto verifier = jwt::verify<jwt::traits::nlohmann_json>().allow_algorithm(jwt::algorithm::rs256{public_key});

            verifier.verify(decoded);

            JwtClaims claims;
            claims.sub = decoded.get_subject();

            for (auto const& [key, val] : decoded.get_payload_json())
            {
                claims.all.emplace(key, val);
            }

            return claims;
        } catch (...)
        {
            return std::nullopt;
        }
    }

}
