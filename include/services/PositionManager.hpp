#pragma once

#include <stdexcept>

#include "domain/Position.hpp"

namespace tradeflow {

class PositionManager {
public:
    Position applyBuy(const Position& current,
                      std::int64_t quantity,
                      std::int64_t executionPrice) const {
        if (quantity <= 0) {
            throw std::invalid_argument("Buy quantity must be positive");
        }
        if (executionPrice <= 0) {
            throw std::invalid_argument("Execution price must be positive");
        }

        Position updated = current;
        updated.userId = current.userId;
        updated.symbol = current.symbol;
        updated.quantity = current.quantity + quantity;

        if (current.quantity == 0) {
            updated.averagePrice = executionPrice;
        } else {
            const std::int64_t oldTotal = current.quantity * current.averagePrice;
            const std::int64_t newTotal = quantity * executionPrice;
            updated.averagePrice = (oldTotal + newTotal) / updated.quantity;
        }

        updated.updatedAt = std::chrono::system_clock::now();
        return updated;
    }

    Position applySell(const Position& current,
                       std::int64_t quantity,
                       std::int64_t executionPrice) const {
        if (quantity <= 0) {
            throw std::invalid_argument("Sell quantity must be positive");
        }
        if (current.quantity < quantity) {
            throw std::invalid_argument("Sell quantity exceeds current position");
        }
        if (executionPrice <= 0) {
            throw std::invalid_argument("Execution price must be positive");
        }

        Position updated = current;
        const std::int64_t realized = quantity * (executionPrice - current.averagePrice);
        updated.realizedPnl += realized;
        updated.quantity = current.quantity - quantity;
        if (updated.quantity == 0) {
            updated.averagePrice = 0;
        }
        updated.updatedAt = std::chrono::system_clock::now();
        return updated;
    }
};

}  // namespace tradeflow
