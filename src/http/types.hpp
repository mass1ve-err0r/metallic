//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <cstdint>


namespace Metallic
{
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    namespace http = beast::http;

    using tcp = asio::ip::tcp;
    using error_code = boost::system::error_code;

    using RequestBody = http::vector_body<std::uint8_t>;
    using ResponseBody = http::string_body;

    using Request = http::request<RequestBody>;
    using Response = http::response<ResponseBody>;

    constexpr auto token = asio::as_tuple(asio::use_awaitable);
}
