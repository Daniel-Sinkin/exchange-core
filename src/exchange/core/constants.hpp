
// pba/core/constants.hpp
#pragma once
//
#include "exchange/core/core_types.hpp"

#include <limits>
//

namespace ds_exch
{
//
// Memory
//
inline constexpr usize k_kib{1024zu};
inline constexpr usize k_mib{1024zu * k_kib};
//
// Core
//
inline constexpr OrderId k_invalid_order_id{std::numeric_limits<OrderId>::max()};

namespace detail
{  // Guardrails and invariants
static_assert(k_kib > 0zu);
static_assert(k_mib > 0zu);
static_assert((k_mib % k_kib) == 0zu);
}  // namespace detail
}  // namespace ds_exch
