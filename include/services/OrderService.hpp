#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "domain/Order.hpp"
#include "domain/Position.hpp"
#include "domain/RiskLimit.hpp"
#include "infrastructure/Logger.hpp"
#include "interfaces/IExecutionEngine.hpp"
#include "interfaces/IOrderRepository.hpp"
#include "interfaces/IPositionRepository.hpp"
#include "interfaces/IRiskManager.hpp"
#include "services/PositionManager.hpp"
#include "services/PnLEngine.hpp"

namespace tradeflow {

struct OrderRequest {
    std::int64_t userId = -1;
    std::string symbol;
    Side side = Side::BUY;
    OrderType orderType = OrderType::LIMIT;
    std::int64_t quantity = 0;
    std::int64_t price = 0;
    std::string idempotencyKey;
};

class OrderService {
public:
    OrderService(IOrderRepository& orderRepository,
                 IPositionRepository& positionRepository,
                 IRiskManager& riskManager,
                 IExecutionEngine& executionEngine,
                 PositionManager positionManager,
                 PnLEngine& pnlEngine,
                 const Logger& logger);

    std::optional<Order> createOrder(const OrderRequest& request);
    std::vector<Order> listOrders(std::int64_t userId = -1, const std::string& symbol = "") const;
    std::optional<Order> getOrder(std::int64_t orderId) const;
    bool cancelOrder(std::int64_t orderId);

    std::vector<Position> listPositions(std::int64_t userId = -1) const;
    std::optional<Position> getPosition(std::int64_t userId, const std::string& symbol) const;
    RiskLimit getRiskLimit(std::int64_t userId) const;
    void upsertRiskLimit(const RiskLimit& riskLimit);
    PnLReport getPnlReport(std::int64_t userId, const std::string& symbol) const;

private:
    IOrderRepository& orderRepository_;
    IPositionRepository& positionRepository_;
    IRiskManager& riskManager_;
    IExecutionEngine& executionEngine_;
    PositionManager positionManager_;
    PnLEngine& pnlEngine_;
    const Logger& logger_;
    mutable std::mutex mutex_;
};

}  // namespace tradeflow
