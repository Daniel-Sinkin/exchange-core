
// pba/core/core_types.hpp
#pragma once

#include "exchange/core/gsl.hpp"

#include <assert.h>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string_view>
#include <type_traits>

namespace ds_exch
{
using usize = std::size_t;
using isize = std::ptrdiff_t;
using iptr = std::intptr_t;
using uptr = std::uintptr_t;

using i64 = std::int64_t;
using i32 = std::int32_t;
using i16 = std::int16_t;
using i8 = std::int8_t;

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u16 = std::uint16_t;
using u8 = std::uint8_t;

#if defined(__cpp_lib_stdfloat) && __cpp_lib_stdfloat >= 202207L
using f32 = std::float32_t;
using f64 = std::float64_t;
#else
using f32 = float;
using f64 = double;
#endif
static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

// C++ Core Guidelines recommend using std::byte rather than
// u8 here so I'll just stick to that.
using Byte = std::byte;

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::duration<f64>;

using OrderId = u64;

using Price = i64;
using Quantity = u64;

enum class Symbol
{
    AAPL,
};
[[nodiscard]] constexpr auto to_string(Symbol symbol) noexcept -> std::string_view
{
    switch (symbol)
    {
        case Symbol::AAPL:
            return "AAPL";
    }
    return "UnknownSymbol";
}

enum class OrderType : u8
{
    LimitBuy,
    LimitSell,
};
[[nodiscard]] constexpr auto to_string(OrderType type) noexcept -> std::string_view
{
    switch (type)
    {
        case OrderType::LimitBuy:
            return "LimitBuy";
        case OrderType::LimitSell:
            return "LimitSell";
    }
    return "UnknownOrderType";
}
inline auto is_buy(OrderType type) -> bool
{
    return (type == OrderType::LimitBuy);
}

enum class TimeInForce : u8
{
    DAY,
    GTC
};
[[nodiscard]] constexpr auto to_string(TimeInForce tif) noexcept -> std::string_view
{
    switch (tif)
    {
        case TimeInForce::DAY:
            return "DAY";
        case TimeInForce::GTC:
            return "GTC";
    }
    return "UnknownTimeInForce";
}

template <typename T>
inline constexpr bool k_always_false = false;

template <typename Enum>
[[nodiscard]] constexpr auto from_string(std::string_view in) -> std::optional<Enum>
{
    if constexpr (std::is_same_v<Enum, Symbol>)
    {
        if (in == "AAPL")
        {
            return Symbol::AAPL;
        }
        return std::nullopt;
    }
    else if constexpr (std::is_same_v<Enum, OrderType>)
    {
        if (in == "LimitBuy")
        {
            return OrderType::LimitBuy;
        }
        if (in == "LimitSell")
        {
            return OrderType::LimitSell;
        }
        return std::nullopt;
    }
    else if constexpr (std::is_same_v<Enum, TimeInForce>)
    {
        if (in == "DAY")
        {
            return TimeInForce::DAY;
        }
        if (in == "GTC")
        {
            return TimeInForce::GTC;
        }
        return std::nullopt;
    }
    else
    {
        static_assert(
            k_always_false<Enum>, "from_string only supports Symbol, OrderType, TimeInForce."
        );
    }
}

template <typename T>
    requires(std::unsigned_integral<T>)
[[nodiscard]] constexpr auto is_power_of_two(T x) noexcept -> bool
{
    return x != T{0} && (x & (x - T{1})) == T{0};
}
}  // namespace ds_exch
