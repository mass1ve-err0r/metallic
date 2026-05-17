//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include <spdlog/spdlog.h>

#include <memory>


namespace Metallic::Logging
{

    class Logger
    {
    public:
        static void init();
        static void shutdown();

        static std::shared_ptr<spdlog::logger> get();

    private:
        static constexpr auto name = "server";
    };

}
