#include <gtest/gtest.h>

#include <optional>

#include "execution/SimulatedExecutionEngine.hpp"
#include "infrastructure/Logger.hpp"
#include "repository/InMemoryOrderRepository.hpp"
#include "repository/InMemoryPositionRepository.hpp"
#include "risk/RiskManager.hpp"
#include "services/OrderService.hpp"

using namespace tradeflow;

TEST(OrderServiceTest, SuccessfulOrder) {
    InMemoryOrderRepository orderRepo;
    InMemoryPositionRepository positionRepo;
    positionRepo.addUser(1);
    positionRepo.upsertRiskLimit({1, 1000, 1000, 1000000});

    RiskManager riskManager;
    SimulatedExecutionEngine executionEngine(1500);
    PositionManager positionManager;
    Logger logger;

    class FixedPriceProvider : public IMarketPriceProvider {
    public:
        std::int64_t getCurrentPrice(const std::string&) const override { return 1500; }
    } provider;
    PnLEngine pnlEngine(provider);
    OrderService service(orderRepo, positionRepo, riskManager, executionEngine, positionManager, pnlEngine, logger);

    OrderRequest request{1, "RELIANCE", Side::BUY, OrderType::LIMIT, 100, 1500, "demo-001"};
    auto order = service.createOrder(request);

    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::FILLED);
    EXPECT_EQ(order->quantity, 100);
}

TEST(OrderServiceTest, RejectedOrder) {
    InMemoryOrderRepository orderRepo;
    InMemoryPositionRepository positionRepo;
    positionRepo.addUser(1);
    positionRepo.upsertRiskLimit({1, 100, 100, 100000});

    RiskManager riskManager;
    SimulatedExecutionEngine executionEngine(1500);
    PositionManager positionManager;
    Logger logger;

    class FixedPriceProvider : public IMarketPriceProvider {
    public:
        std::int64_t getCurrentPrice(const std::string&) const override { return 1500; }
    } provider;
    PnLEngine pnlEngine(provider);
    OrderService service(orderRepo, positionRepo, riskManager, executionEngine, positionManager, pnlEngine, logger);

    OrderRequest request{1, "RELIANCE", Side::BUY, OrderType::LIMIT, 101, 1500, "demo-002"};
    auto order = service.createOrder(request);

    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::REJECTED);
}

TEST(OrderServiceTest, DuplicateIdempotencyKey) {
    InMemoryOrderRepository orderRepo;
    InMemoryPositionRepository positionRepo;
    positionRepo.addUser(1);
    positionRepo.upsertRiskLimit({1, 1000, 1000, 1000000});

    RiskManager riskManager;
    SimulatedExecutionEngine executionEngine(1500);
    PositionManager positionManager;
    Logger logger;

    class FixedPriceProvider : public IMarketPriceProvider {
    public:
        std::int64_t getCurrentPrice(const std::string&) const override { return 1500; }
    } provider;
    PnLEngine pnlEngine(provider);
    OrderService service(orderRepo, positionRepo, riskManager, executionEngine, positionManager, pnlEngine, logger);

    OrderRequest request{1, "RELIANCE", Side::BUY, OrderType::LIMIT, 50, 1500, "duplicate-001"};
    auto first = service.createOrder(request);
    auto second = service.createOrder(request);

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(first->id, second->id);
}
