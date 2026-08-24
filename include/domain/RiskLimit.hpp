#pragma once

#include <cstdint>

namespace tradeflow {

struct RiskLimit {
    std::int64_t userId = -1;
    std::int64_t maxOrderQuantity = 0;
    std::int64_t maxPositionQuantity = 0;
    std::int64_t maxNotional = 0;
};

}  // namespace tradeflow
