// app/main.cpp
#include "exchange/core/constants.hpp"
#include "exchange/core/core_types.hpp"

#include <print>

namespace ds_exch
{
struct Order
{
    OrderId id{k_invalid_order_id};
    Symbol symbol{Symbol::_invalid};
};

auto my_func() -> void
{
    std::println("Hello, World");
}
}  // namespace ds_exch

int main()
{
    using namespace ::ds_exch;
    my_func();
    return 0;
}
