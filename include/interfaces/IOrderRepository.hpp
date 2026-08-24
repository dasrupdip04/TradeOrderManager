#pragma once

#include <optional>
#include <string>
#include <vector>

#include "domain/Execution.hpp"
#include "domain/Order.hpp"

namespace tradeflow {

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;

    virtual std::optional<Order> findById(std::int64_t orderId) const = 0;
    virtual std::optional<Order> findByIdempotencyKey(std::int64_t userId,
                                                     const std::string& key) const = 0;
    virtual std::vector<Order> listOrders(std::int64_t userId = -1,
                                         const std::string& symbol = "") const = 0;
    virtual std::vector<Execution> listExecutionsForOrder(std::int64_t orderId) const = 0;
    virtual std::optional<Order> insert(const Order& order) = 0;
    virtual bool insertExecution(const Execution& execution) = 0;
    virtual bool updateStatus(std::int64_t orderId, OrderStatus status) = 0;
    virtual bool cancel(std::int64_t orderId) = 0;
};

}  // namespace tradeflow
