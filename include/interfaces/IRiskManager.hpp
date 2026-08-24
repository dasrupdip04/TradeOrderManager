#pragma once

#include <string>

#include "domain/Order.hpp"
#include "domain/Position.hpp"
#include "domain/RiskLimit.hpp"

namespace tradeflow {

struct RiskCheckResult {
    bool allowed = false;
    std::string reason;
};

class IRiskManager {
public:
    virtual ~IRiskManager() = default;

    virtual RiskCheckResult validate(const Order& order,
                                     const RiskLimit& limit,
                                     const Position& currentPosition,
                                     bool userExists) const = 0;
};

}  // namespace tradeflow
