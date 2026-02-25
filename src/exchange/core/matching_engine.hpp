#pragma once

#include "exchange/core/constants.hpp"
#include "exchange/core/core_types.hpp"

#include <limits>
#include <map>
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
    Quantity qty;
    TimeInForce time_in_force;
    u64 sender_id;
    u64 timestamp;
};

struct OrderDebugView
{
    u64 timestamp;
    OrderId order_id;
    Quantity qty;
};

struct PriceLevelDebugView
{
    Price price;
    std::vector<OrderDebugView> orders;
};

struct OrderBookDebugSnapshot
{
    Symbol symbol;
    std::vector<PriceLevelDebugView> buy_levels;
    std::vector<PriceLevelDebugView> sell_levels;
};

struct Trade
{
    OrderId buy_id;
    OrderId sell_id;
    Price price;
    Quantity qty;
    u64 timestamp;
    u64 trade_seq;
};

using TradeHistory = std::vector<Trade>;

class MatchingEngine
{
  public:
    explicit MatchingEngine(Symbol symbol);

    [[nodiscard]] auto symbol() const -> Symbol;
    [[nodiscard]] auto num_buy_levels() const -> usize;
    [[nodiscard]] auto num_sell_levels() const -> usize;
    [[nodiscard]] auto active_trade_history_snapshot() const -> TradeHistory;

    auto submit_order(const Order& order) -> bool;
    auto match() -> void;
    [[nodiscard]] auto debug_snapshot(usize max_levels = std::numeric_limits<usize>::max()) const
        -> OrderBookDebugSnapshot;

  private:
    std::map<Price, std::queue<Order>, std::greater<Price>> buy_levels_map_{};
    std::map<Price, std::queue<Order>, std::less<Price>> sell_levels_map_{};
    TradeHistory active_trade_history_tape_{};
    Symbol symbol_;
    u64 current_trade_seq_{0};
};

}  // namespace ds_exch
