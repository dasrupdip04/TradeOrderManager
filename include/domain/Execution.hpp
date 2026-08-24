#pragma once

#include <chrono>
#include <cstdint>

namespace tradeflow {

struct Execution {
    std::int64_t id = -1;
    std::int64_t orderId = -1;
    std::int64_t executedQuantity = 0;
    std::int64_t executionPrice = 0;
    std::chrono::system_clock::time_point executedAt{};
};

}  // namespace tradeflow
