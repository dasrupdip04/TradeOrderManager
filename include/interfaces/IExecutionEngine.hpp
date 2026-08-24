#pragma once

#include "domain/Execution.hpp"
#include "domain/Order.hpp"

namespace tradeflow {

class IExecutionEngine {
public:
    virtual ~IExecutionEngine() = default;

    virtual Execution execute(const Order& order) const = 0;
};

}  // namespace tradeflow
