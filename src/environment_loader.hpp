//
// Created by Saadat Baig on 19.05.26.
//
// Loads a declared set of environment variables at startup, falling back to
// provided default values. All values are stored in a read-only concurrent-
// safe map after initialisation.
//
// Usage:
//
//   // Declare the variables you want at startup (key, env-var name, default)
//   static const EnvironmentConfiguration cfg({
//       {"app.keystore.path", "APP_KEYSTORE_PATH", ""},
//       {"app.keystore.pass", "APP_KEYSTORE_PASS", "peppol"},
//       {"app.server.port",   "APP_PORT",          "8090"},
//   });
//
//   std::string path = cfg.get("app.keystore.path");
//   uint16_t    port = cfg.get_as<uint16_t>("app.server.port");
//
// Thread safety:
//   The map is populated once in the constructor and never mutated afterward.
//   All reads are safe from any thread with no locking required.
//
#pragma once

#include <charconv>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


// ---------------------------------------------------------------------------
// EnvironmentVariable — declaration of one environment variable
// ---------------------------------------------------------------------------
struct EnvironmentVariable {
    std::string key;          // internal lookup key used in code
    std::string env_name;     // the actual environment variable name (e.g. "AP_PORT")
    std::string default_value; // used if the variable is not set or is empty
};

// ---------------------------------------------------------------------------
// EnvironmentConfiguration
// ---------------------------------------------------------------------------
class EnvironmentConfiguration
{
public:
    // Loads all declared variables from the environment.
    // Missing or empty variables use their declared default.
    explicit EnvironmentConfiguration(std::vector<EnvironmentVariable> declarations)
    {
        for (auto& decl : declarations)
        {
            const char* raw = std::getenv(decl.env_name.c_str());
            std::string value = (raw && raw[0] != '\0') ? raw : decl.default_value;

            map_.emplace(std::move(decl.key), std::move(value));
        }
    }

    // Returns the value for `key` as a string.
    // Throws std::out_of_range if `key` was not declared.
    const std::string& get(std::string_view key) const
    {
        auto it = map_.find(std::string(key));
        if (it == map_.end())
        {
            throw std::out_of_range("EnvironmentConfiguration: key not declared: " + std::string(key));
        }

        return it->second;
    }

    // Returns the value for `key` converted to T.
    // Supported types: all integer types, float, double, bool ("true"/"1").
    // Throws std::invalid_argument if the value cannot be converted.
    template <typename T>
    T get_as(std::string_view key) const
    {
        const std::string& val = get(key);
        return _convert<T>(val, key);
    }

    // Returns the value as std::optional<std::string>.
    // Returns nullopt if the stored value is the empty string.
    std::optional<std::string> get_optional(std::string_view key) const
    {
        const std::string& val = get(key);
        if (val.empty()) return std::nullopt;

        return val;
    }

    // Returns true if the stored value is non-empty.
    bool is_set(std::string_view key) const
    {
        return !get(key).empty();
    }

private:
    // Read-only after construction — no mutex needed.
    std::unordered_map<std::string, std::string> map_;

    // ---------------------------------------------------------------------------
    // Type conversion helpers
    // ---------------------------------------------------------------------------

    template <typename T>
    static T _convert(const std::string& val, std::string_view key)
    {
        if constexpr (std::is_same_v<T, std::string>)
        {
            return val;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            return val == "true" || val == "1" || val == "yes";
        }
        else if constexpr (std::is_integral_v<T>)
        {
            T result{};
            auto [ptr, ec] = std::from_chars(val.data(), val.data() + val.size(), result);
            if (ec != std::errc{} || ptr != val.data() + val.size())
            {
                throw std::invalid_argument("EnvironmentConfiguration: cannot convert \"" + val + "\" to integer for key: " + std::string(key));
            }

            return result;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            // std::from_chars for float/double is C++17 but not widely
            // available on all platforms — fall back to stod/stof.
            try
            {
                size_t pos = 0;
                if constexpr (std::is_same_v<T, float>)
                {
                    return std::stof(val, &pos);
                }
                else
                {
                    return static_cast<T>(std::stod(val, &pos));
                }
            } catch (...)
            {
                throw std::invalid_argument("EnvironmentConfiguration: cannot convert \"" + val + "\" to float for key: " + std::string(key));
            }
        }
        else
        {
            static_assert(!sizeof(T), "EnvironmentConfiguration::get_as<T>: unsupported type T");
        }

    }

};
