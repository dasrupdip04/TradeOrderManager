#include "services/OrderService.hpp"

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <utility>

namespace tradeflow {

OrderService::OrderService(IOrderRepository& orderRepository,
                           IPositionRepository& positionRepository,
                           IRiskManager& riskManager,
                           IExecutionEngine& executionEngine,
                           PositionManager positionManager,
                           PnLEngine& pnlEngine,
                           const Logger& logger)
    : orderRepository_(orderRepository),
      positionRepository_(positionRepository),
      riskManager_(riskManager),
      executionEngine_(executionEngine),
      positionManager_(std::move(positionManager)),
      pnlEngine_(pnlEngine),
      logger_(logger) {}

std::optional<Order> OrderService::createOrder(const OrderRequest& request) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (request.userId <= 0) {
        throw std::invalid_argument("userId must be positive");
    }
    if (request.symbol.empty()) {
        throw std::invalid_argument("symbol cannot be empty");
    }
    if (request.idempotencyKey.empty()) {
        throw std::invalid_argument("idempotencyKey is required");
    }

    auto existing = orderRepository_.findByIdempotencyKey(request.userId, request.idempotencyKey);
    if (existing.has_value()) {
        logger_.info("Duplicate idempotency key accepted for user " + std::to_string(request.userId));
        return existing;
    }

    Position currentPosition = positionRepository_.getPosition(request.userId, request.symbol).value_or(Position{request.userId, request.symbol, 0, 0, 0, std::chrono::system_clock::now()});
    RiskLimit limit = positionRepository_.getRiskLimit(request.userId);
    const bool userExists = positionRepository_.userExists(request.userId);

    Order order;
    order.userId = request.userId;
    order.symbol = request.symbol;
    order.side = request.side;
    order.orderType = request.orderType;
    order.quantity = request.quantity;
    order.price = request.price;
    order.status = OrderStatus::NEW;
    order.idempotencyKey = request.idempotencyKey;
    auto now = std::chrono::system_clock::now();
    order.createdAt = now;
    order.updatedAt = now;

    const auto riskResult = riskManager_.validate(order, limit, currentPosition, userExists);
    if (!riskResult.allowed) {
        order.status = OrderStatus::REJECTED;
        logger_.warn("Risk rejection for user " + std::to_string(request.userId) + ": " + riskResult.reason);
        return order;
    }

    order.status = OrderStatus::ACCEPTED;
    auto persisted = orderRepository_.insert(order);
    if (!persisted.has_value()) {
        throw std::runtime_error("Failed to persist order");
    }
    order = *persisted;

    auto execution = executionEngine_.execute(order);
    if (orderRepository_.insertExecution(execution)) {
        logger_.info("Executed order " + std::to_string(order.id) + " for " + order.symbol);
    }

    if (order.side == Side::BUY) {
        Position nextPosition = positionManager_.applyBuy(currentPosition, order.quantity, execution.executionPrice);
        nextPosition.userId = order.userId;
        nextPosition.symbol = order.symbol;
        positionRepository_.upsertPosition(nextPosition);
    } else {
        Position nextPosition = positionManager_.applySell(currentPosition, order.quantity, execution.executionPrice);
        nextPosition.userId = order.userId;
        nextPosition.symbol = order.symbol;
        positionRepository_.upsertPosition(nextPosition);
    }

    order.status = OrderStatus::FILLED;
    orderRepository_.updateStatus(order.id, OrderStatus::FILLED);
    logger_.info("Position updated for user " + std::to_string(order.userId) + " symbol " + order.symbol);
    return orderRepository_.findById(order.id);
}

std::vector<Order> OrderService::listOrders(std::int64_t userId, const std::string& symbol) const {
    return orderRepository_.listOrders(userId, symbol);
}

std::optional<Order> OrderService::getOrder(std::int64_t orderId) const {
    return orderRepository_.findById(orderId);
}

bool OrderService::cancelOrder(std::int64_t orderId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto order = orderRepository_.findById(orderId);
    if (!order.has_value()) {
        return false;
    }
    logger_.info("Cancelling order " + std::to_string(orderId));
    return orderRepository_.cancel(orderId);
}

std::vector<Position> OrderService::listPositions(std::int64_t userId) const {
    return positionRepository_.listPositions(userId);
}

std::optional<Position> OrderService::getPosition(std::int64_t userId, const std::string& symbol) const {
    return positionRepository_.getPosition(userId, symbol);
}

RiskLimit OrderService::getRiskLimit(std::int64_t userId) const {
    return positionRepository_.getRiskLimit(userId);
}

void OrderService::upsertRiskLimit(const RiskLimit& riskLimit) {
    positionRepository_.upsertRiskLimit(riskLimit);
}

PnLReport OrderService::getPnlReport(std::int64_t userId, const std::string& symbol) const {
    auto position = positionRepository_.getPosition(userId, symbol);
    if (!position.has_value()) {
        return {0, 0, 0};
    }
    return pnlEngine_.calculateTotalPnl(*position, symbol);
}

}  // namespace tradeflow
