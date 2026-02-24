// exchange/core/matching_engine.hpp
#pragma once

#include "exchange/core/constants.hpp"
#include "exchange/core/core_types.hpp"
#include "exchange/core/pch.hpp"

#include <algorithm>
#include <format>
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
    MatchingEngine(Symbol symbol) : symbol_(symbol)
    {
        trade_history_tape_.reserve(1024);
    }

    [[nodiscard]] auto symbol() const -> Symbol
    {
        return symbol_;
    }

    [[nodiscard]] auto num_buy_levels() const -> usize
    {
        return buy_levels_map_.size();
    }

    [[nodiscard]] auto num_sell_levels() const -> usize
    {
        return sell_levels_map_.size();
    }

    [[nodiscard]] auto trade_history_reference() const -> const TradeHistory&
    {
        return trade_history_tape_;
    }

    [[nodiscard]] auto trade_history_snapshot() const -> TradeHistory
    {
        return trade_history_tape_;
    }

    auto submit_order(const Order& order) -> bool
    {
        if (order.symbol != symbol_)
        {
            return false;
        }
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
        return true;
    }

    auto match() -> void
    {
        while (!buy_levels_map_.empty() && !sell_levels_map_.empty())
        {
            auto buy_level_it = buy_levels_map_.begin();
            auto sell_level_it = sell_levels_map_.begin();

            if (buy_level_it->first < sell_level_it->first)
            {
                break;
            }

            auto& buy_level_queue = buy_level_it->second;
            auto& sell_level_queue = sell_level_it->second;

            if (buy_level_queue.empty())
            {
                buy_levels_map_.erase(buy_level_it);
                continue;
            }
            if (sell_level_queue.empty())
            {
                sell_levels_map_.erase(sell_level_it);
                continue;
            }

            auto& buy_order = buy_level_queue.front();
            auto& sell_order = sell_level_queue.front();

            const auto fill_qty = std::min(buy_order.qty, sell_order.qty);
            buy_order.qty -= fill_qty;
            sell_order.qty -= fill_qty;
            trade_history_tape_.emplace_back(
                buy_order.id,
                sell_order.id,
                sell_order.price,
                fill_qty,
                std::max(buy_order.timestamp, sell_order.timestamp),
                current_trade_seq_++
            );

            if (buy_order.qty == 0u)
            {
                buy_level_queue.pop();
            }
            if (sell_order.qty == 0u)
            {
                sell_level_queue.pop();
            }

            if (buy_level_queue.empty())
            {
                buy_levels_map_.erase(buy_level_it);
            }
            if (sell_level_queue.empty())
            {
                sell_levels_map_.erase(sell_level_it);
            }
        }
    }

    [[nodiscard]] auto debug_snapshot(usize max_levels = std::numeric_limits<usize>::max()) const
        -> OrderBookDebugSnapshot
    {
        return OrderBookDebugSnapshot{
            .symbol = symbol_,
            .buy_levels = snapshot_side_levels(buy_levels_map_, max_levels),
            .sell_levels = snapshot_side_levels(sell_levels_map_, max_levels),
        };
    }

  private:
    template <typename PriceComparator>
    [[nodiscard]] static auto snapshot_side_levels(
        const std::map<Price, std::queue<Order>, PriceComparator>& levels, usize max_levels
    ) -> std::vector<PriceLevelDebugView>
    {
        std::vector<PriceLevelDebugView> snapshots{};
        snapshots.reserve(std::min(max_levels, levels.size()));

        for (const auto& [price, level_queue] : levels)
        {
            if (snapshots.size() >= max_levels)
            {
                break;
            }

            auto level_queue_copy{level_queue};
            std::vector<OrderDebugView> order_snapshots{};
            order_snapshots.reserve(level_queue_copy.size());

            while (!level_queue_copy.empty())
            {
                const auto& order = level_queue_copy.front();
                order_snapshots.push_back(
                    OrderDebugView{
                        .timestamp = order.timestamp,
                        .order_id = order.id,
                        .qty = order.qty,
                    }
                );
                level_queue_copy.pop();
            }

            snapshots.push_back(
                PriceLevelDebugView{
                    .price = price,
                    .orders = std::move(order_snapshots),
                }
            );
        }

        return snapshots;
    }

    std::map<Price, std::queue<Order>, std::greater<Price>> buy_levels_map_{};
    std::map<Price, std::queue<Order>, std::less<Price>> sell_levels_map_{};
    std::vector<Trade> trade_history_tape_{};

    Symbol symbol_;

    u64 current_trade_seq_{0};
};
}  // namespace ds_exch

template <>
struct std::formatter<ds_exch::Order>
{
    constexpr auto parse(std::format_parse_context& ctx) -> std::format_parse_context::iterator
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error("invalid format specifier for ds_exch::Order");
        }
        return it;
    }

    auto format(const ds_exch::Order& order, std::format_context& ctx) const
        -> std::format_context::iterator
    {
        return std::format_to(
            ctx.out(),
            "Order{{id={}, symbol={}, type={}, price={}, quantity={}, tif={}, sender_id={}, "
            "timestamp={}}}",
            order.id,
            ds_exch::to_string(order.symbol),
            ds_exch::to_string(order.type),
            order.price,
            order.qty,
            ds_exch::to_string(order.time_in_force),
            order.sender_id,
            order.timestamp
        );
    }
};

template <>
struct std::formatter<ds_exch::MatchingEngine>
{
    constexpr auto parse(std::format_parse_context& ctx) -> std::format_parse_context::iterator
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error("invalid format specifier for ds_exch::MatchingEngine");
        }
        return it;
    }

    auto format(const ds_exch::MatchingEngine& engine, std::format_context& ctx) const
        -> std::format_context::iterator
    {
        return std::format_to(
            ctx.out(),
            "MatchingEngine{{symbol={}, buy_levels={}, sell_levels={}}}",
            ds_exch::to_string(engine.symbol()),
            engine.num_buy_levels(),
            engine.num_sell_levels()
        );
    }
};

template <>
struct std::formatter<ds_exch::Trade>
{
    constexpr auto parse(std::format_parse_context& ctx) -> std::format_parse_context::iterator
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error("invalid format specifier for ds_exch::Trade");
        }
        return it;
    }

    auto format(const ds_exch::Trade& trade, std::format_context& ctx) const
        -> std::format_context::iterator
    {
        return std::format_to(
            ctx.out(),
            "Trade{{buy_id={}, sell_id={}, price={}, qty={}, timestamp={}, trade_seq={}}}",
            trade.buy_id,
            trade.sell_id,
            trade.price,
            trade.qty,
            trade.timestamp,
            trade.trade_seq
        );
    }
};

template <>
struct std::formatter<ds_exch::OrderDebugView>
{
    constexpr auto parse(std::format_parse_context& ctx) -> std::format_parse_context::iterator
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error("invalid format specifier for ds_exch::OrderDebugView");
        }
        return it;
    }

    auto format(const ds_exch::OrderDebugView& order, std::format_context& ctx) const
        -> std::format_context::iterator
    {
        return std::format_to(
            ctx.out(), "({}, {}, {})", order.timestamp, order.order_id, order.qty
        );
    }
};

template <>
struct std::formatter<ds_exch::PriceLevelDebugView>
{
    constexpr auto parse(std::format_parse_context& ctx) -> std::format_parse_context::iterator
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error("invalid format specifier for ds_exch::PriceLevelDebugView");
        }
        return it;
    }

    auto format(const ds_exch::PriceLevelDebugView& level, std::format_context& ctx) const
        -> std::format_context::iterator
    {
        auto out = std::format_to(ctx.out(), "{}\n", level.price);

        for (const auto& order : level.orders)
        {
            out = std::format_to(out, "  - {}\n", order);
        }

        return out;
    }
};

template <>
struct std::formatter<ds_exch::OrderBookDebugSnapshot>
{
    constexpr auto parse(std::format_parse_context& ctx) -> std::format_parse_context::iterator
    {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error("invalid format specifier for ds_exch::OrderBookDebugSnapshot");
        }
        return it;
    }

    auto format(const ds_exch::OrderBookDebugSnapshot& snapshot, std::format_context& ctx) const
        -> std::format_context::iterator
    {
        auto out = std::format_to(ctx.out(), "{}\nBUY\n", ds_exch::to_string(snapshot.symbol));

        if (snapshot.buy_levels.empty())
        {
            out = std::format_to(out, "  (empty)\n");
        }
        else
        {
            for (const auto& level : snapshot.buy_levels)
            {
                out = std::format_to(out, "{}", level);
            }
        }

        out = std::format_to(out, "SELL\n");

        if (snapshot.sell_levels.empty())
        {
            out = std::format_to(out, "  (empty)");
        }
        else
        {
            for (const auto& level : snapshot.sell_levels)
            {
                out = std::format_to(out, "{}", level);
            }
        }

        return out;
    }
};
