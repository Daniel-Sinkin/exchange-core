// app/main.cpp
#include "exchange/core/constants.hpp"
#include "exchange/core/core_types.hpp"
#include "exchange/core/matching_engine.hpp"
#include "exchange/core/pch.hpp"

#include <print>
#include <queue>
#include <vector>

namespace ds_exch
{
}  // namespace ds_exch

int main()
{
    using namespace ::ds_exch;

    MatchingEngine me{Symbol::AAPL};

    return 0;
}
