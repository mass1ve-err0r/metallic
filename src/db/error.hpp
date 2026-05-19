//
// Created by Saadat Baig on 19.05.26.
//
#pragma once

#include <stdexcept>
#include <string>


namespace Metallic::Persistence
{
    class DbError : public std::runtime_error
    {
    public:
        explicit DbError(std::string const& msg) : std::runtime_error{msg} {}
    };
}
