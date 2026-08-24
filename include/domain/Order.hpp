#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#include "domain/enums.hpp"

namespace tradeflow {

struct Order {
    std::int64_t id = -1;
    std::int64_t userId = -1;
    std::string symbol;
    Side side = Side::BUY;
    OrderType orderType = OrderType::LIMIT;
    std::int64_t quantity = 0;
    std::int64_t price = 0;
    OrderStatus status = OrderStatus::NEW;
    std::string idempotencyKey;
    std::chrono::system_clock::time_point createdAt{};
    std::chrono::system_clock::time_point updatedAt{};
};

}  // namespace tradeflow
