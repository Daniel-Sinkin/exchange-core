
// pba/core/core_types.hpp
#pragma once

#include "exchange/core/gsl.hpp"

#include <assert.h>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

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
[[nodiscard]] auto to_string(Symbol symbol) -> std::string
{
    switch (symbol)
    {
        case Symbol::AAPL:
            return "AAPL";
    }
};

enum class OrderType : u8
{
    LimitBuy,
    LimitSell,
};
inline auto is_buy(OrderType type) -> bool
{
    return (type == OrderType::LimitBuy);
}

enum class TimeInForce : u8
{
    DAY,
    GTC
};

template <typename T>
    requires(std::unsigned_integral<T>)
[[nodiscard]] constexpr auto is_power_of_two(T x) noexcept -> bool
{
    return x != T{0} && (x & (x - T{1})) == T{0};
}
}  // namespace ds_exch
