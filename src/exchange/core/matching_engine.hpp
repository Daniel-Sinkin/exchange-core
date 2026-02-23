// app/main.cpp
#include "exchange/core/constants.hpp"
#include "exchange/core/core_types.hpp"
#include "exchange/core/pch.hpp"

#include <print>
#include <queue>
#include <vector>

namespace ds_exch
{

struct Order
{
    OrderId id;
    Symbol symbol;
    OrderType type;
    Price price;
    Quantity quantity;
    TimeInForce time_in_force;
    u64 sender_id;
    u64 timestamp;
};

struct OrderMaxHeapSorter
{
    [[nodiscard]] auto operator()(const Order& a, const Order& b) const noexcept -> bool
    {
        return a.timestamp < b.timestamp;
    };
};
using OrderMaxHeap = std::priority_queue<Order, std::vector<Order>, OrderMaxHeapSorter>;

struct OrderMinHeapSorter
{
    [[nodiscard]] auto operator()(const Order& a, const Order& b) const noexcept -> bool
    {
        return a.timestamp > b.timestamp;
    };
};
using OrderMinHeap = std::priority_queue<Order, std::vector<Order>, OrderMinHeapSorter>;

class MatchingEngine
{
  public:
    MatchingEngine(Symbol symbol) : symbol_(symbol)
    {
    }

    auto symbol() const -> Symbol
    {
        return symbol_;
    }

    auto get_num_buy_levels() const -> usize
    {
        return buy_levels_map_.size();
    }

    auto get_num_sell_levels() const -> usize
    {
        return sell_levels_map_.size();
    }

    auto submit_order(const Order& order) -> void
    {
        if (is_buy(order.type))
        {
            if (auto it = buy_levels_map_.find(order.price); it != buy_levels_map_.end())
            {
                it->second.push(order);
            }
            else
            {
                buy_levels_map_[order.price].push({order});
            }
        }
        else
        {
            if (auto it = sell_levels_map_.find(order.price); it != sell_levels_map_.end())
            {
                it->second.push(order);
            }
            else
            {
                sell_levels_map_[order.price].push({order});
            }
        }
    }

  private:
    std::map<Price, OrderMaxHeap> buy_levels_map_{};
    std::map<Price, OrderMaxHeap> sell_levels_map_{};

    Symbol symbol_;
};
}  // namespace ds_exch
