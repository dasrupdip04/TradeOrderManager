#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "domain/Execution.hpp"
#include "domain/Order.hpp"
#include "interfaces/IOrderRepository.hpp"

namespace tradeflow {

class InMemoryOrderRepository : public IOrderRepository {
public:
    InMemoryOrderRepository() = default;

    std::optional<Order> findById(std::int64_t orderId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& order : orders_) {
            if (order.id == orderId) {
                return order;
            }
        }
        return std::nullopt;
    }

    std::optional<Order> findByIdempotencyKey(std::int64_t userId,
                                             const std::string& key) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& order : orders_) {
            if (order.userId == userId && order.idempotencyKey == key) {
                return order;
            }
        }
        return std::nullopt;
    }

    std::vector<Order> listOrders(std::int64_t userId,
                                 const std::string& symbol) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Order> results;
        for (const auto& order : orders_) {
            if (userId != -1 && order.userId != userId) {
                continue;
            }
            if (!symbol.empty() && order.symbol != symbol) {
                continue;
            }
            results.push_back(order);
        }
        return results;
    }

    std::vector<Execution> listExecutionsForOrder(std::int64_t orderId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Execution> results;
        for (const auto& execution : executions_) {
            if (execution.orderId == orderId) {
                results.push_back(execution);
            }
        }
        return results;
    }

    std::optional<Order> insert(const Order& order) override {
        std::lock_guard<std::mutex> lock(mutex_);
        Order persisted = order;
        if (persisted.id <= 0) {
            persisted.id = ++nextOrderId_;
        }
        orders_.push_back(persisted);
        return persisted;
    }

    bool updateStatus(std::int64_t orderId, OrderStatus status) override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& order : orders_) {
            if (order.id == orderId) {
                order.status = status;
                order.updatedAt = std::chrono::system_clock::now();
                return true;
            }
        }
        return false;
    }

    bool cancel(std::int64_t orderId) override {
        return updateStatus(orderId, OrderStatus::CANCELLED);
    }

    bool insertExecution(const Execution& execution) override {
        std::lock_guard<std::mutex> lock(mutex_);
        Execution persisted = execution;
        if (persisted.id <= 0) {
            persisted.id = ++nextExecutionId_;
        }
        executions_.push_back(persisted);
        return true;
    }

private:
    mutable std::mutex mutex_;
    std::vector<Order> orders_;
    std::vector<Execution> executions_;
    std::int64_t nextOrderId_ = 0;
    std::int64_t nextExecutionId_ = 0;
};

}  // namespace tradeflow
