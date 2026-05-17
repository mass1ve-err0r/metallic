//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include "types.hpp"

#include <string>


namespace Metallic
{

    inline Response
    json_response(Request const& req, http::status status, std::string body)
    {
        Response res{status, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = std::move(body);
        res.prepare_payload();
        return res;
    }

    inline Response
    text_response(Request const& req, http::status status, std::string body)
    {
        Response res{status, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/plain; charset=utf-8");
        res.keep_alive(req.keep_alive());
        res.body() = std::move(body);
        res.prepare_payload();
        return res;
    }

    inline Response
    not_found(Request const& req)
    {
        return json_response(req, http::status::not_found, R"({"error":"not_found"})");
    }

    inline Response
    method_not_allowed(Request const& req)
    {
        return json_response(req, http::status::method_not_allowed, R"({"error":"method_not_allowed"})");
    }

}
