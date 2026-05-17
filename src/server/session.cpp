//
// Created by Saadat Baig on 17.05.26.
//
#include "session.hpp"

#include <spdlog/spdlog.h>

#include <chrono>


namespace Metallic::Server
{

    asio::awaitable<void>
    session(tcp::socket socket, std::shared_ptr<App const> app)
    {
        auto remote = socket.remote_endpoint();

        beast::tcp_stream stream{std::move(socket)};
        beast::flat_buffer buffer;

        spdlog::debug(
            "accepted connection from {}:{}",
            remote.address().to_string(),
            remote.port()
        );

        for (;;) {
            stream.expires_after(std::chrono::seconds{30});

            Request req;

            auto [read_ec, bytes_read] =
                co_await http::async_read(stream, buffer, req, token);

            static_cast<void>(bytes_read);

            if (read_ec == http::error::end_of_stream) {
                break;
            }

            if (read_ec) {
                spdlog::debug(
                    "read failed from {}:{}: {}",
                    remote.address().to_string(),
                    remote.port(),
                    read_ec.message()
                );
                co_return;
            }

            RequestContext ctx{
                .request = req,
                .remote_endpoint = remote
            };

            auto res = co_await app->handle(ctx);
            bool const should_close = res.need_eof();

            spdlog::info(
                "{} {} -> {} body={}B",
                req.method_string(),
                req.target(),
                res.result_int(),
                ctx.body_bytes().size()
            );

            auto [write_ec, bytes_written] =
                co_await http::async_write(stream, res, token);

            static_cast<void>(bytes_written);

            if (write_ec) {
                spdlog::debug(
                    "write failed to {}:{}: {}",
                    remote.address().to_string(),
                    remote.port(),
                    write_ec.message()
                );
                co_return;
            }

            if (should_close) {
                break;
            }
        }

        error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_send, ec);

        if (ec) {
            spdlog::debug(
                "shutdown failed for {}:{}: {}",
                remote.address().to_string(),
                remote.port(),
                ec.message()
            );
        }
    }

}
