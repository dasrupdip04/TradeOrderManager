#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace tradeflow {

struct Position {
    std::int64_t userId = -1;
    std::string symbol;
    std::int64_t quantity = 0;
    std::int64_t averagePrice = 0;
    std::int64_t realizedPnl = 0;
    std::chrono::system_clock::time_point updatedAt{};
};

}  // namespace tradeflow
