#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "execution/SimulatedExecutionEngine.hpp"
#include "infrastructure/Logger.hpp"
#include "repository/InMemoryOrderRepository.hpp"
#include "repository/InMemoryPositionRepository.hpp"
#include "risk/RiskManager.hpp"
#include "services/OrderService.hpp"

using namespace tradeflow;

TEST(ConcurrencyTest, ConcurrentOrdersRespectRiskLimit) {
    InMemoryOrderRepository orderRepo;
    InMemoryPositionRepository positionRepo;
    positionRepo.addUser(1);
    positionRepo.upsertRiskLimit({1, 1000, 1000, 1000000});

    RiskManager riskManager;
    SimulatedExecutionEngine executionEngine(1000);
    PositionManager positionManager;
    Logger logger;

    class FixedPriceProvider : public IMarketPriceProvider {
    public:
        std::int64_t getCurrentPrice(const std::string&) const override { return 1000; }
    } provider;
    PnLEngine pnlEngine(provider);
    OrderService service(orderRepo, positionRepo, riskManager, executionEngine, positionManager, pnlEngine, logger);

    std::vector<std::thread> threads;
    threads.reserve(8);
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&service]() {
            OrderRequest request{1, "TCS", Side::BUY, OrderType::MARKET, 200, 1000, "conc-" + std::to_string(std::rand())};
            service.createOrder(request);
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto positions = service.listPositions(1);
    std::int64_t total = 0;
    for (const auto& position : positions) {
        total += position.quantity;
    }
    EXPECT_LE(total, 1000);
}
