//
// Created by Saadat Baig on 19.05.26.
//
#pragma once

#include "error.hpp"

#include <boost/pfr.hpp>
#include <pqxx/pqxx>

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>


namespace Metallic::Persistence
{

    // Converts a pqxx field to the target type T
    template<typename T>
    T field_cast(pqxx::field const& f)
    {
        if (f.is_null()) {
            throw DbError{"unexpected NULL for non-optional field: " + std::string{f.name()}};
        }
        return f.as<T>();
    }

    template<typename T>
    std::optional<T> field_cast_opt(pqxx::field const& f)
    {
        if (f.is_null()) return std::nullopt;
        return f.as<T>();
    }

    namespace detail
    {
        // Populate each field of aggregate T from a pqxx::row, matched by column name
        template<typename T, std::size_t... I>
        void from_row_impl(T& obj, pqxx::row const& row, std::array<std::string_view, sizeof...(I)> const& names, std::index_sequence<I...>)
        {
            ((boost::pfr::get<I>(obj) = field_cast<
                std::remove_reference_t<decltype(boost::pfr::get<I>(obj))>
            >(row[std::string{names[I]}])), ...);
        }

        // Build a pqxx param list from all fields of T
        template<typename T, std::size_t... I>
        std::vector<std::string> to_params_impl(T const& obj, std::index_sequence<I...>)
        {
            std::vector<std::string> params;
            params.reserve(sizeof...(I));
            ((params.push_back(pqxx::to_string(boost::pfr::get<I>(obj)))), ...);
            return params;
        }
    }

    template<typename Derived>
    struct Entity
    {
        // Populate a Derived instance from a pqxx::row
        static Derived from_row(pqxx::row const& row)
        {
            Derived obj{};
            constexpr auto names = Derived::field_names();
            constexpr auto N = std::tuple_size_v<decltype(names)>;
            static_assert(
                boost::pfr::tuple_size_v<Derived> == N,
                "METALLIC_ENTITY field count does not match struct field count"
            );
            detail::from_row_impl(obj, row, names, std::make_index_sequence<N>{});
            return obj;
        }

        // Returns all field values as strings for use in pqxx params
        std::vector<std::string> to_params() const
        {
            constexpr auto N = boost::pfr::tuple_size_v<Derived>;
            return detail::to_params_impl(static_cast<Derived const&>(*this), std::make_index_sequence<N>{});
        }
    };

}

// Declares field names for an Entity struct.
// Usage inside the struct body: METALLIC_ENTITY(field1, field2, ...)
// Generates a static field_names() and a compile-time count check.
#define METALLIC_ENTITY(...)                                                        \
    static constexpr auto field_names()                                             \
    {                                                                               \
        using sv = std::string_view;                                                \
        constexpr std::array names = { METALLIC_DETAIL_MAP_SV(__VA_ARGS__) };      \
        return names;                                                               \
    }

// Expands each name to a string_view literal — internal helper
#define METALLIC_DETAIL_SV(x) std::string_view{#x}
#define METALLIC_DETAIL_MAP_SV(...) METALLIC_DETAIL_EXPAND(METALLIC_DETAIL_MAP_INNER(__VA_ARGS__))

// Variadic map — supports up to 16 fields (extend if needed)
#define METALLIC_DETAIL_EXPAND(x) x
#define METALLIC_DETAIL_MAP_INNER(...)                                              \
    METALLIC_DETAIL_PICK(                                                           \
        __VA_ARGS__,                                                                \
        METALLIC_DETAIL_M16, METALLIC_DETAIL_M15, METALLIC_DETAIL_M14,             \
        METALLIC_DETAIL_M13, METALLIC_DETAIL_M12, METALLIC_DETAIL_M11,             \
        METALLIC_DETAIL_M10, METALLIC_DETAIL_M9,  METALLIC_DETAIL_M8,              \
        METALLIC_DETAIL_M7,  METALLIC_DETAIL_M6,  METALLIC_DETAIL_M5,              \
        METALLIC_DETAIL_M4,  METALLIC_DETAIL_M3,  METALLIC_DETAIL_M2,              \
        METALLIC_DETAIL_M1                                                          \
    )(__VA_ARGS__)

#define METALLIC_DETAIL_PICK(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,NAME,...) NAME

#define METALLIC_DETAIL_M1(a) METALLIC_DETAIL_SV(a)
#define METALLIC_DETAIL_M2(a,b) METALLIC_DETAIL_SV(a),METALLIC_DETAIL_SV(b)
#define METALLIC_DETAIL_M3(a,b,c) METALLIC_DETAIL_M2(a,b),METALLIC_DETAIL_SV(c)
#define METALLIC_DETAIL_M4(a,b,c,d) METALLIC_DETAIL_M3(a,b,c),METALLIC_DETAIL_SV(d)
#define METALLIC_DETAIL_M5(a,b,c,d,e) METALLIC_DETAIL_M4(a,b,c,d),METALLIC_DETAIL_SV(e)
#define METALLIC_DETAIL_M6(a,b,c,d,e,f) METALLIC_DETAIL_M5(a,b,c,d,e),METALLIC_DETAIL_SV(f)
#define METALLIC_DETAIL_M7(a,b,c,d,e,f,g) METALLIC_DETAIL_M6(a,b,c,d,e,f),METALLIC_DETAIL_SV(g)
#define METALLIC_DETAIL_M8(a,b,c,d,e,f,g,h) METALLIC_DETAIL_M7(a,b,c,d,e,f,g),METALLIC_DETAIL_SV(h)
#define METALLIC_DETAIL_M9(a,b,c,d,e,f,g,h,i) METALLIC_DETAIL_M8(a,b,c,d,e,f,g,h),METALLIC_DETAIL_SV(i)
#define METALLIC_DETAIL_M10(a,b,c,d,e,f,g,h,i,j) METALLIC_DETAIL_M9(a,b,c,d,e,f,g,h,i),METALLIC_DETAIL_SV(j)
#define METALLIC_DETAIL_M11(a,b,c,d,e,f,g,h,i,j,k) METALLIC_DETAIL_M10(a,b,c,d,e,f,g,h,i,j),METALLIC_DETAIL_SV(k)
#define METALLIC_DETAIL_M12(a,b,c,d,e,f,g,h,i,j,k,l) METALLIC_DETAIL_M11(a,b,c,d,e,f,g,h,i,j,k),METALLIC_DETAIL_SV(l)
#define METALLIC_DETAIL_M13(a,b,c,d,e,f,g,h,i,j,k,l,m) METALLIC_DETAIL_M12(a,b,c,d,e,f,g,h,i,j,k,l),METALLIC_DETAIL_SV(m)
#define METALLIC_DETAIL_M14(a,b,c,d,e,f,g,h,i,j,k,l,m,n) METALLIC_DETAIL_M13(a,b,c,d,e,f,g,h,i,j,k,l,m),METALLIC_DETAIL_SV(n)
#define METALLIC_DETAIL_M15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) METALLIC_DETAIL_M14(a,b,c,d,e,f,g,h,i,j,k,l,m,n),METALLIC_DETAIL_SV(o)
#define METALLIC_DETAIL_M16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p) METALLIC_DETAIL_M15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o),METALLIC_DETAIL_SV(p)
