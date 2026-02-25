#include "exchange/core/core_types.hpp"
#include "exchange/core/matching_engine.hpp"
#include "exchange/core/pch.hpp"

#include <print>

namespace ds_exch
{

}

int main()
{
    using namespace ds_exch;

    MatchingEngine me{Symbol::AAPL};
    me.print();
}
