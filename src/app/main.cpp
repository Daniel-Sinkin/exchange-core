// app/main.cpp
#include "exchange/core/pch.hpp"  // IWYU pragma: keep
//
#include "exchange/core/constants.hpp"
#include "exchange/core/core_types.hpp"
#include "exchange/core/matching_engine.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <optional>
#include <print>
#include <string>

namespace ds_exch
{
auto parse_u64(const std::string& token) -> std::optional<u64>
{
    try
    {
        return static_cast<u64>(std::stoull(token));
    }
    catch (...)
    {
        return std::nullopt;
    }
}
auto parse_i64(const std::string& token) -> std::optional<i64>
{
    try
    {
        return static_cast<i64>(std::stoll(token));
    }
    catch (...)
    {
        return std::nullopt;
    }
}

auto parse_order_from_tokens(const std::vector<std::string>& tokens) -> std::optional<Order>
{
    if (tokens.size() != 8zu)
    {
        std::println(std::cerr, "Wrong number of tokens");
        return std::nullopt;
    }
    const auto order_id_res = parse_u64(tokens[0]);
    if (!order_id_res)
    {
        std::println(std::cerr, "Failed to parse order_id");
        return std::nullopt;
    }
    const auto symbol_res = from_string<Symbol>(tokens[1]);
    if (!symbol_res)
    {
        std::println(std::cerr, "Failed to parse symbol");
        return std::nullopt;
    }
    const auto order_type_res = from_string<OrderType>(tokens[2]);
    if (!order_type_res)
    {
        std::println(std::cerr, "Failed to parse order_type");
        return std::nullopt;
    }
    const auto price_res = parse_i64(tokens[3]);
    if (!price_res)
    {
        std::println(std::cerr, "Failed to parse price");
        return std::nullopt;
    }
    const auto qty_res = parse_u64(tokens[4]);
    if (!qty_res || *qty_res == 0)
    {
        std::println(std::cerr, "Failed to parse quantity");
        return std::nullopt;
    }
    const auto tif_res = from_string<TimeInForce>(tokens[5]);
    if (!tif_res)
    {
        std::println(std::cerr, "Failed to parse tif");
        return std::nullopt;
    }
    const auto sender_id_res = parse_u64(tokens[6]);
    if (!sender_id_res)
    {
        std::println(std::cerr, "Failed to parse sender_id");
        return std::nullopt;
    }
    const auto timestamp_res = parse_u64(tokens[7]);
    if (!timestamp_res)
    {
        std::println(std::cerr, "Failed to parse timestamp");
        return std::nullopt;
    }
    return Order{
        .id = *order_id_res,
        .symbol = *symbol_res,
        .type = *order_type_res,
        .price = *price_res,
        .qty = *qty_res,
        .time_in_force = *tif_res,
        .sender_id = *sender_id_res,
        .timestamp = *timestamp_res
    };
}

}  // namespace ds_exch

int main()
{
    using namespace ds_exch;

    std::ios::sync_with_stdio(false);
    const std::string trade_history_path{"tests/data/trade_history.csv"};
    {
        std::ofstream trade_history_file{trade_history_path, std::ios::trunc};
        if (!trade_history_file.is_open())
        {
            std::println(
                std::cerr, "Failed to open '{}' for truncation before run.", trade_history_path
            );
            return 1;
        }
    }

    const std::vector<Order> orders = []
    {
        std::vector<Order> out{};
        out.reserve(128);
        auto is_first = true;
        while (true)
        {
            std::string line;
            std::getline(std::cin, line);
            if (line.empty())
            {
                return out;
            }
            if (is_first)
            {
                is_first = false;
                continue;
            }
            std::vector<std::string> tokens{};
            tokens.reserve(8);
            std::string token;
            std::istringstream ss{line};
            while (std::getline(ss, token, k_input_delimiter))
            {
                tokens.push_back(std::move(token));
            }
            std::println("{}", tokens);
            if (auto order_res = parse_order_from_tokens(tokens); order_res)
            {
                out.push_back(*order_res);
            }
        }
        return out;
    }();

    MatchingEngine me{Symbol::AAPL, trade_history_path};
    for (const auto& order : orders)
    {
        if (!me.submit_order(order))
        {
            std::println(std::cerr, "Failed to submit order: {}", order);
            std::terminate();
        }
    }

    std::println("Before matching: {}\n{}", me, me.debug_snapshot());
    me.match();
    std::println("After matching: {}\n{}", me, me.debug_snapshot());
    std::println("Trade History:");
    const auto trades = me.active_trade_history_snapshot();
    if (trades.empty())
    {
        std::println("  (empty)");
    }
    else
    {
        for (const auto& trade : trades)
        {
            std::println("  - {}", trade);
        }
    }
    return 0;
}
