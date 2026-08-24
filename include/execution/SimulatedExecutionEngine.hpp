#pragma once

#include <chrono>
#include <cstdint>
#include <stdexcept>

#include "domain/Execution.hpp"
#include "domain/Order.hpp"
#include "interfaces/IExecutionEngine.hpp"

namespace tradeflow {

class SimulatedExecutionEngine : public IExecutionEngine {
public:
    explicit SimulatedExecutionEngine(std::int64_t marketPrice = 1000)
        : marketPrice_(marketPrice) {}

    Execution execute(const Order& order) const override {
        if (order.quantity <= 0) {
            throw std::invalid_argument("Order quantity must be positive");
        }

        auto executionPrice = order.price;
        if (order.orderType == OrderType::MARKET) {
            executionPrice = marketPrice_;
        }

        Execution execution;
        execution.orderId = order.id;
        execution.executedQuantity = order.quantity;
        execution.executionPrice = executionPrice;
        execution.executedAt = std::chrono::system_clock::now();
        return execution;
    }

private:
    std::int64_t marketPrice_;
};

}  // namespace tradeflow
