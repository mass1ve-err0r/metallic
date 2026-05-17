//
// Created by Saadat Baig on 17.05.26.
//
#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>


namespace Metallic
{

    class BodyView
    {
    public:
        explicit BodyView(std::vector<std::uint8_t> const& body): body_{body} {}

        [[nodiscard]] bool
        empty() const noexcept {
            return body_.empty();
        }

        [[nodiscard]] std::size_t
        size() const noexcept
        {
            return body_.size();
        }

        [[nodiscard]] std::uint8_t const*
        data() const noexcept
        {
            return body_.data();
        }

        [[nodiscard]] std::span<std::uint8_t const>
        bytes() const noexcept
        {
            return body_;
        }

        [[nodiscard]] std::string_view
        as_string_view() const noexcept
        {
            return {
                reinterpret_cast<char const*>(body_.data()),
                body_.size()
            };
        }

        [[nodiscard]] std::string
        to_string() const
        {
            auto view = as_string_view();
            return std::string{view.data(), view.size()};
        }

    private:
        std::vector<std::uint8_t> const& body_;
    };

}
