//
// Created by Saadat Baig on 17.05.26.
//
#include "logger.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>


namespace Metallic::Logging
{

    void
    Logger::init()
    {
        spdlog::init_thread_pool(8192, 1);

        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [tid %t] %v");

        auto logger = std::make_shared<spdlog::async_logger>(
            name,
            std::move(sink),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::overrun_oldest
        );

        logger->set_level(spdlog::level::info);
        logger->flush_on(spdlog::level::warn);

        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
    }

    void
    Logger::shutdown()
    {
        spdlog::shutdown();
    }

    std::shared_ptr<spdlog::logger>
    Logger::get()
    {
        return spdlog::get(name);
    }

}
