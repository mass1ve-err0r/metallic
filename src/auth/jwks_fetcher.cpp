//
// Created by Saadat Baig on 19.05.26.
//
#include "jwks_fetcher.hpp"

#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>


namespace Metallic::Auth
{
    namespace
    {
        std::size_t
        write_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata)
        {
            auto* buf = static_cast<std::string *>(userdata);
            buf->append(ptr, size * nmemb);

            return size * nmemb;
        }

        std::string
        http_get(std::string const& url)
        {
            CURL* curl = curl_easy_init();
            if (!curl)
            {
                throw std::runtime_error("curl_easy_init failed");
            }

            std::string body;

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK)
            {
                throw std::runtime_error(std::string{"curl fetch failed: "} + curl_easy_strerror(res));
            }

            return body;
        }
    }

    JwksKeySet
    fetch_jwks(std::string const& issuer)
    {
        auto const url = issuer + "/protocol/openid-connect/certs";
        auto const body = http_get(url);

        std::cout << "got JWKS: " << body << "\n";

        try
        {
            return jwt::parse_jwks<jwt::traits::nlohmann_json>(body);
        } catch (std::exception const& e)
        {
            throw std::runtime_error(std::string{"failed to parse JWKS: "} + e.what());
        }
    }

}
