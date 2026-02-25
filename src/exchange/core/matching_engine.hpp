#pragma once
#include "exchange/core/pch.hpp"  // IWYU pragma: keep
//
#include "exchange/core/core_types.hpp"

#include <print>

namespace ds_exch
{
struct Order
{
    OrderId id;
    OrderType type;
    Price price;
    Quantity qty;
    TimeInForce time_in_force;
};
struct Trade
{
    OrderId buy_id;
    OrderId sell_id;
    Price price;
    Quantity qty;
    u64 trade_seq;
    u64 timestamp;
};

class MatchingEngine
{
  public:
    MatchingEngine(Symbol symbol) : symbol_(symbol)
    {
    }

    [[nodiscard]] auto symbol() const noexcept -> Symbol
    {
        return symbol_;
    }

    auto submit_order(const Order& order) -> void
    {
    }

    auto print() const -> void
    {
        std::println("Matching Engine for Symbol {}", to_string(symbol_));
        for (const auto& [price, queue] : buy_orders_)
        {
            std::println("{}", price);
            std::println(" - {}, {}, {}", queue.front().id, queue.front().price, queue.front().qty);
        }
        for (const auto& [price, queue] : sell_orders_)
        {
            std::println("{}", price);
            std::println(" - {}, {}, {}", queue.front().id, queue.front().price, queue.front().qty);
        }
    }

  private:
    Symbol symbol_;
    std::map<Price, std::queue<Order>, std::less<Price>> buy_orders_;
    std::map<Price, std::queue<Order>, std::greater<Price>> sell_orders_;
};

}  // namespace ds_exch
